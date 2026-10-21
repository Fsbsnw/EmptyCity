#include "Subsystem/ECGameFlowSubsystem.h"

#include "ECGameplayTags.h"
#include "Character/Enemy/ECEnemyBoss.h"
#include "Character/Player/ECPlayer.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "Component/Attribute/ECHealthComponent.h"
#include "Data/Item/ECItemTableRow.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "Engine/LocalPlayer.h"
#include "Equipment/ECQuickBarComponent.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "Save/ECSaveSubsystem.h"
#include "Subsystem/ECCutsceneSubsystem.h"
#include "Subsystem/ECItemDataSubsystem.h"
#include "Subsystem/ECRegionSubsystem.h"
#include "Subsystem/ECTimeSubsystem.h"
#include "UI/Subsystem/UIManagerSubsystem.h"

void UECGameFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // SaveSubsystem이 먼저 준비된 뒤 GameFlow가 동작하도록 의존성을 명시한다.
    Collection.InitializeDependency<UECSaveSubsystem>();
    Collection.InitializeDependency<UECGameplayMessageSubsystem>();
	Collection.InitializeDependency<UECRegionSubsystem>();
    Collection.InitializeDependency<UECTimeSubsystem>();

    UECGameplayMessageSubsystem& MessageSubsystem = UECGameplayMessageSubsystem::Get(this);

    PlayerDeathListenerHandle = MessageSubsystem.RegisterListener(
        TAG_Message_Player_Death,
        this,
        &ThisClass::HandlePlayerDeath);
	BossHalfHealthListenerHandle = MessageSubsystem.RegisterListener(
		TAG_Message_Boss_HalfHealthReached,
		this,
		&ThisClass::HandleBossHalfHealthReached);
	BossDeathListenerHandle = MessageSubsystem.RegisterListener(
		TAG_Message_Boss_Death,
		this,
		&ThisClass::HandleBossDeath);

    UECTimeSubsystem& TimeSubsystem = UECTimeSubsystem::Get(this);
    TimeSubsystem.OnGameTimeChanged.AddUObject(
        this,
        &ThisClass::HandleGameTimeChanged);

	UECRegionSubsystem& RegionSubsystem = UECRegionSubsystem::Get(this);
	RegionSubsystem.OnRegionChanged.AddUObject(
		this,
		&ThisClass::HandleRegionChanged);
}

void UECGameFlowSubsystem::Deinitialize()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UECTimeSubsystem* TimeSubsystem = GameInstance->GetSubsystem<UECTimeSubsystem>())
        {
            TimeSubsystem->OnGameTimeChanged.RemoveAll(this);
        }

		if (UECRegionSubsystem* RegionSubsystem = GameInstance->GetSubsystem<UECRegionSubsystem>())
		{
			RegionSubsystem->OnRegionChanged.RemoveAll(this);
		}
    }

    PlayerDeathListenerHandle.Unregister();
	BossHalfHealthListenerHandle.Unregister();
	BossDeathListenerHandle.Unregister();

    Super::Deinitialize();
}

bool UECGameFlowSubsystem::SaveMorningCheckpoint()
{
    UECSaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UECSaveSubsystem>();

    if (!SaveSubsystem)
    {
        return false;
    }

    // GameFlow는 "지금 저장한다"는 정책만 결정한다.
    // 실제 런타임 데이터 수집과 저장은 SaveSubsystem
    return SaveSubsystem->SaveCurrentGame();
}

void UECGameFlowSubsystem::HandleGameTimeChanged(int32 Day, EECTimeOfDay TimeOfDay)
{
	const UECSaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UECSaveSubsystem>();

	// 사망 체크포인트를 복원하면서 시간이 아침으로 바뀐 경우에는
	// 로드 도중의 상태를 다시 저장하지 않는다.
	if (bDeathFlowInProgress
		|| (SaveSubsystem && SaveSubsystem->IsLoadInProgress())
		|| UECTimeSubsystem::Get(this).IsApplyingTransientDebugTime()
		|| TimeOfDay != EECTimeOfDay::Morning)
	{
		return;
	}

	if (!SaveMorningCheckpoint())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save morning checkpoint for day %d"), Day);
    }
}

void UECGameFlowSubsystem::HandleRegionChanged(
	FGameplayTag PreviousRegionTag,
	FGameplayTag NewRegionTag)
{
	const bool bEnteredCultistBossBattle =
		PreviousRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_CultistBase)
		&& NewRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_CultistBase_BossBattle);

	if (!bEnteredCultistBossBattle)
	{
		return;
	}

	UECSaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UECSaveSubsystem>();
	if (!SaveSubsystem || !SaveSubsystem->SaveCurrentGame())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save Cultist boss battle checkpoint."));
	}
}

void UECGameFlowSubsystem::HandlePlayerDeath(FGameplayTag Channel, const FECPlayerDeathMessage& Message)
{
    if (bDeathFlowInProgress)
    {
        return;
    }

    bDeathFlowInProgress = true;
	CurrentPlayerDeathMessage = Message;
	DeathRegionTag = UECRegionSubsystem::Get(this).GetCurrentRegionTag();
	LoseRandomPlayerInventoryItem(CurrentPlayerDeathMessage);

	AECPlayerController* PlayerController = Cast<AECPlayerController>(Message.PlayerController);
    UUIManagerSubsystem* UIManager = IsValid(PlayerController)
        ? UUIManagerSubsystem::Get(PlayerController)
        : nullptr;
    if (UIManager)
    {
		PlayerController->SetIgnoreMoveInput(true);
		PlayerController->SetIgnoreLookInput(true);

		// 사망 어빌리티의 지연이 끝날 때 CompletePlayerDeathFlow가 복귀를 실행합니다.
        UIManager->RequestToggleInput(
            ECGameplayTags::InputTag_UI_DeathScreen,
            Message.DamageInstigator);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[GameFlow] Player death UI manager is unavailable."));
    }
}

void UECGameFlowSubsystem::CompletePlayerDeathFlow(AECPlayer* Player)
{
	AECPlayerController* PlayerController =
		Cast<AECPlayerController>(CurrentPlayerDeathMessage.PlayerController);
	if (!bDeathFlowInProgress
		|| !IsValid(PlayerController)
		|| !IsValid(Player)
		|| CurrentPlayerDeathMessage.PlayerActor != Player)
	{
		UE_LOG(LogTemp, Error, TEXT("[GameFlow] 사망 복귀 대상 플레이어를 찾을 수 없습니다."));
		bDeathFlowInProgress = false;
		return;
	}

	const bool bBossBattleDeath = DeathRegionTag.MatchesTagExact(
		ECGameplayTags::MapNode_Location_CultistBase_BossBattle);
	const FGameplayTag RespawnRegionTag = bBossBattleDeath
		? ECGameplayTags::MapNode_Location_CultistBase_BossBattle
		: ECGameplayTags::MapNode_Location_Shelter;

	if (!bBossBattleDeath)
	{
		UECTimeSubsystem::Get(this).SleepUntilTomorrow();
	}
	else
	{
		RestoreBossHealthForRetry();
	}

	Player->RecoverFromDeath();
	const bool bRespawnSucceeded =
		UECRegionSubsystem::Get(this).RespawnAtRegion(RespawnRegionTag);

	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(PlayerController))
	{
		if (UIManager->GetActiveWidget(ECGameplayTags::InputTag_UI_DeathScreen))
		{
			UIManager->RequestToggleInput(
				ECGameplayTags::InputTag_UI_DeathScreen,
				Player);
		}
	}

	PlayerController->ResetIgnoreMoveInput();
	PlayerController->ResetIgnoreLookInput();
	DeathRegionTag = FGameplayTag();
	bDeathFlowInProgress = false;

	if (!bRespawnSucceeded)
	{
		UE_LOG(LogTemp, Error, TEXT("[GameFlow] 사망 복귀 텔레포트에 실패했습니다: %s"),
			*RespawnRegionTag.ToString());
		return;
	}

	if (!SaveMorningCheckpoint())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameFlow] 사망 복귀 상태 저장에 실패했습니다."));
	}
}

void UECGameFlowSubsystem::RestoreBossHealthForRetry()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	for (TActorIterator<AECEnemyBoss> It(World); It; ++It)
	{
		AECEnemyBoss* Boss = *It;
		// 시퀀서용 복제 액터가 아니라 AIController가 빙의한 실제 전투 보스를 대상으로 합니다.
		if (!IsValid(Boss) || !IsValid(Boss->GetController()))
		{
			continue;
		}

		if (UECHealthComponent* BossHealth = UECHealthComponent::FindHealthComponent(Boss))
		{
			Boss->ResetBossPatternForRetry();
			BossHealth->RestoreHealthToMax();
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[GameFlow] 재도전할 실제 보스 액터를 찾지 못했습니다."));
}

void UECGameFlowSubsystem::LoseRandomPlayerInventoryItem(FECPlayerDeathMessage& InOutMessage)
{
	AECPlayerController* PlayerController = Cast<AECPlayerController>(InOutMessage.PlayerController);
	UECInventoryManagerComponent* Inventory = IsValid(PlayerController)
		? PlayerController->GetInventoryManagerComponent()
		: nullptr;
	if (!IsValid(Inventory))
	{
		return;
	}

	TArray<const FInventoryEntry*> LossCandidates;
	UECItemDataSubsystem& ItemDataSubsystem = UECItemDataSubsystem::Get(this);
	for (const FInventoryEntry& Entry : Inventory->InventoryList.Entries)
	{
		if (!IsValid(Entry.Instance) || Entry.StackCount <= 0)
		{
			continue;
		}

		const FECItemTableRow* ItemData = ItemDataSubsystem.FindItemData(Entry.Instance);
		if (ItemData && ItemData->bCanDiscard && !ItemData->bPermanentPlayerItem)
		{
			LossCandidates.Add(&Entry);
		}
	}

	if (LossCandidates.IsEmpty())
	{
		return;
	}

	const FInventoryEntry* SelectedEntry =
		LossCandidates[FMath::RandRange(0, LossCandidates.Num() - 1)];
	UECInventoryItemInstance* SelectedItem = SelectedEntry->Instance;
	const TSubclassOf<UECInventoryItemDefinition> ItemDefinition = SelectedItem->ItemDef;
	const int32 StackCount = SelectedEntry->StackCount;

	const int32 RemovedCount = Inventory->RemoveItemCount(SelectedItem, StackCount);
	if (RemovedCount <= 0)
	{
		return;
	}

	if (UECQuickBarComponent* QuickBar = PlayerController->GetQuickBarComponent())
	{
		const TArray<UECInventoryItemInstance*> QuickBarSlots = QuickBar->GetSlots();
		for (int32 SlotIndex = 0; SlotIndex < QuickBarSlots.Num(); ++SlotIndex)
		{
			if (QuickBarSlots[SlotIndex] == SelectedItem)
			{
				QuickBar->RemoveItemFromSlot(SlotIndex);
			}
		}
	}

	InOutMessage.DroppedItemDefinition = ItemDefinition;
	InOutMessage.DroppedItemCount = RemovedCount;
}

void UECGameFlowSubsystem::HandleBossHalfHealthReached(
	FGameplayTag Channel,
	const FECBossHalfHealthReachedMessage& Message)
{
	UE_LOG(
		LogTemp,
		Log,
		TEXT("[GameFlow] Boss half-health event received. Boss=%s, HealthRatio=%.3f, Cutscene=%s"),
		*GetNameSafe(Message.BossActor),
		Message.HealthRatio,
		*Message.CutsceneTag.ToString());

	if (!Message.CutsceneTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameFlow] Boss half-health event has no valid cutscene tag."));
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	ULocalPlayer* LocalPlayer = GameInstance ? GameInstance->GetFirstGamePlayer() : nullptr;
	UECCutsceneSubsystem* CutsceneSubsystem = LocalPlayer
		? LocalPlayer->GetSubsystem<UECCutsceneSubsystem>()
		: nullptr;

	if (!IsValid(CutsceneSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameFlow] CutsceneSubsystem is unavailable."));
		return;
	}

	if (!CutsceneSubsystem->PlayCutsceneByTag(Message.CutsceneTag, Message.BossActor))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GameFlow] Failed to start boss phase cutscene: %s"),
			*Message.CutsceneTag.ToString());
	}
}

void UECGameFlowSubsystem::HandleBossDeath(
	FGameplayTag Channel,
	const FECBossDeathMessage& Message)
{
	UE_LOG(
		LogTemp,
		Log,
		TEXT("[GameFlow] Boss death event received. Boss=%s, Cutscene=%s"),
		*GetNameSafe(Message.BossActor),
		*Message.CutsceneTag.ToString());

	if (!Message.CutsceneTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameFlow] Boss death event has no valid cutscene tag."));
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	ULocalPlayer* LocalPlayer = GameInstance ? GameInstance->GetFirstGamePlayer() : nullptr;
	UECCutsceneSubsystem* CutsceneSubsystem = LocalPlayer
		? LocalPlayer->GetSubsystem<UECCutsceneSubsystem>()
		: nullptr;

	if (!IsValid(CutsceneSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameFlow] CutsceneSubsystem is unavailable."));
		return;
	}

	if (!CutsceneSubsystem->PlayCutsceneByTag(Message.CutsceneTag, Message.BossActor))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GameFlow] Failed to start boss death cutscene: %s"),
			*Message.CutsceneTag.ToString());
	}
}
