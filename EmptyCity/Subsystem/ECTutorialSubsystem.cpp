#include "Subsystem/ECTutorialSubsystem.h"

#include "ECGameplayTags.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "Data/Tutorial/ECTutorialSettings.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Save/ECSaveSubsystem.h"
#include "Subsystem/ECMessageTypes.h"
#include "Subsystem/ECRegionSubsystem.h"
#include "Subsystem/ECTimeSubsystem.h"
#include "UI/Subsystem/UIManagerSubsystem.h"
#include "UI/Widget/Tutorial/ECTutorialWidget.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Tutorial, "TAG.UI.Tutorial");

void UECTutorialSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UGameInstance* GameInstance = GetLocalPlayer() ? GetLocalPlayer()->GetGameInstance() : nullptr;
	if (!GameInstance)
	{
		return;
	}

	if (UECGameplayMessageSubsystem* MessageSubsystem =
		GameInstance->GetSubsystem<UECGameplayMessageSubsystem>())
	{
		TutorialInputListenerHandle = MessageSubsystem->RegisterListener(
			TAG_Message_Tutorial_Input,
			this,
			&ThisClass::HandleTutorialInput);
	}

	if (UECRegionSubsystem* RegionSubsystem = GameInstance->GetSubsystem<UECRegionSubsystem>())
	{
		RegionSubsystem->OnRegionChanged.AddUObject(this, &ThisClass::HandleRegionChanged);
	}

	if (UECSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UECSaveSubsystem>())
	{
		SaveSubsystem->RegisterSaveable(this);
	}
}

void UECTutorialSubsystem::Deinitialize()
{
	TutorialInputListenerHandle.Unregister();

	if (UGameInstance* GameInstance = GetLocalPlayer() ? GetLocalPlayer()->GetGameInstance() : nullptr)
	{
		if (UECRegionSubsystem* RegionSubsystem = GameInstance->GetSubsystem<UECRegionSubsystem>())
		{
			RegionSubsystem->OnRegionChanged.RemoveAll(this);
		}

		if (UECSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UECSaveSubsystem>())
		{
			SaveSubsystem->UnregisterSaveable(this);
		}
	}

	TutorialWidget = nullptr;
	Super::Deinitialize();
}

void UECTutorialSubsystem::InitializeTutorialUI()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	AECPlayerController* PlayerController = LocalPlayer
		? Cast<AECPlayerController>(LocalPlayer->GetPlayerController(GetWorld()))
		: nullptr;
	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(PlayerController);
	if (!UIManager)
	{
		return;
	}

	if (UECTutorialWidget* ActiveWidget =
		Cast<UECTutorialWidget>(UIManager->GetActiveWidget(TAG_UI_Tutorial)))
	{
		TutorialWidget = ActiveWidget;
		RefreshTutorialWidget();
		return;
	}

	const UECTutorialSettings* Settings = GetDefault<UECTutorialSettings>();
	TSubclassOf<UECTutorialWidget> WidgetClass = Settings
		? Settings->TutorialWidgetClass.LoadSynchronous()
		: nullptr;
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Tutorial Settings에 TutorialWidgetClass가 설정되지 않았습니다."));
		return;
	}

	FUIConfigWidget WidgetConfig;
	WidgetConfig.WidgetClass = WidgetClass;
	WidgetConfig.CachePolicy = EWidgetCachePolicy::AlwaysCache;
	WidgetConfig.LayerType = EWidgetLayerType::HUD;
	WidgetConfig.bShowMouseCursor = false;
	WidgetConfig.TransitionType = EWidgetTransitionType::None;

	TutorialWidget = Cast<UECTutorialWidget>(UIManager->OpenDynamicWidget(
		TAG_UI_Tutorial,
		WidgetConfig));
	RefreshTutorialWidget();
}

void UECTutorialSubsystem::ResetTutorial()
{
	CurrentStage = EECTutorialStage::Movement;
	bSprintUsed = false;
	bAttackUsed = false;
	bGuardUsed = false;
	RefreshTutorialWidget();
}

void UECTutorialSubsystem::HandleTutorialInput(
	FGameplayTag Channel,
	const FECTutorialInputMessage& Message)
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (Message.PlayerController
		&& LocalPlayer
		&& Message.PlayerController != LocalPlayer->GetPlayerController(GetWorld()))
	{
		return;
	}

	const FGameplayTag InputTag = Message.InputTag;
	switch (CurrentStage)
	{
	case EECTutorialStage::Movement:
		if (InputTag.MatchesTagExact(ECGameplayTags::InputTag_Move))
		{
			SetStage(EECTutorialStage::OpenMap);
		}
		break;

	case EECTutorialStage::OpenMap:
		if (InputTag.MatchesTagExact(ECGameplayTags::InputTag_UI_Map))
		{
			SetStage(EECTutorialStage::AwaitFirstRegionTravel);
		}
		break;

	case EECTutorialStage::Sprint:
		if (InputTag.MatchesTagExact(ECGameplayTags::InputTag_Ability_Sprint))
		{
			bSprintUsed = true;
			SetStage(EECTutorialStage::AttackAndGuard);
		}
		break;

	case EECTutorialStage::AttackAndGuard:
		if (InputTag.MatchesTagExact(ECGameplayTags::InputTag_Ability_Attack_Light)
			|| InputTag.MatchesTagExact(ECGameplayTags::InputTag_Ability_Attack_Heavy))
		{
			bAttackUsed = true;
		}
		else if (InputTag.MatchesTagExact(ECGameplayTags::InputTag_Ability_Guard))
		{
			bGuardUsed = true;
		}

		if (bAttackUsed && bGuardUsed)
		{
			SetStage(EECTutorialStage::Completed);
		}
		else
		{
			RefreshTutorialWidget();
		}
		break;

	default:
		break;
	}
}

void UECTutorialSubsystem::HandleRegionChanged(
	FGameplayTag PreviousRegionTag,
	FGameplayTag NewRegionTag)
{
	if (CurrentStage == EECTutorialStage::AwaitFirstRegionTravel
		&& PreviousRegionTag != NewRegionTag
		&& !NewRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_Shelter)
		&& !NewRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_TradingPost))
	{
		SetStage(EECTutorialStage::Sprint);
	}
}

void UECTutorialSubsystem::SetStage(const EECTutorialStage NewStage)
{
	if (CurrentStage == NewStage)
	{
		return;
	}

	CurrentStage = NewStage;
	RefreshTutorialWidget();
}

void UECTutorialSubsystem::RefreshTutorialWidget()
{
	if (!IsValid(TutorialWidget))
	{
		return;
	}

	bool bPrimaryCompleted = false;
	bool bSecondaryCompleted = false;
	if (CurrentStage == EECTutorialStage::Sprint)
	{
		bPrimaryCompleted = bSprintUsed;
	}
	else if (CurrentStage == EECTutorialStage::AttackAndGuard)
	{
		bPrimaryCompleted = bAttackUsed;
		bSecondaryCompleted = bGuardUsed;
	}

	TutorialWidget->ApplyTutorialState(
		CurrentStage,
		bPrimaryCompleted,
		bSecondaryCompleted);
}

void UECTutorialSubsystem::WriteSaveData(FECGameplaySaveData& InOutSaveData) const
{
	InOutSaveData.Tutorial.bInitialized = true;
	InOutSaveData.Tutorial.Stage = CurrentStage;
	InOutSaveData.Tutorial.bSprintUsed = bSprintUsed;
	InOutSaveData.Tutorial.bAttackUsed = bAttackUsed;
	InOutSaveData.Tutorial.bGuardUsed = bGuardUsed;
}

void UECTutorialSubsystem::ReadSaveData(const FECGameplaySaveData& SaveData)
{
	// 튜토리얼 필드가 없던 기존 세이브가 2일차 이상이면 이미 기본 조작을 익힌 것으로 처리합니다.
	if (!SaveData.Tutorial.bInitialized && SaveData.Time.Day > UECTimeSubsystem::MinDay)
	{
		CurrentStage = EECTutorialStage::Completed;
		bSprintUsed = true;
		bAttackUsed = true;
		bGuardUsed = true;
	}
	else
	{
		CurrentStage = SaveData.Tutorial.Stage;
		if (!StaticEnum<EECTutorialStage>()->IsValidEnumValue(static_cast<int64>(CurrentStage)))
		{
			// Travel Denied가 튜토리얼 단계였던 개발 버전의 세이브는 완료 상태로 정리합니다.
			CurrentStage = EECTutorialStage::Completed;
		}
		bSprintUsed = SaveData.Tutorial.bSprintUsed;
		bAttackUsed = SaveData.Tutorial.bAttackUsed;
		bGuardUsed = SaveData.Tutorial.bGuardUsed;
	}

	RefreshTutorialWidget();
}

