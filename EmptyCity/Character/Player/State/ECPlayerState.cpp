#include "ECPlayerState.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/ECAbilitySet.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "AbilitySystem/ECAbilitySystemGlobals.h"
#include "AbilitySystem/Attribute/ECHealthSet.h"
#include "GameplayAbilitiesModule.h"
#include "AbilitySystem/Attribute/ECCombatSet.h"
#include "AbilitySystem/Attribute/ECStaminaSet.h"
#include "AbilitySystem/Attribute/ECMoveSpeedSet.h"
#include "Actor/InventoryContainer/ECInventoryContainer.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "Data/Workbench/ECWorkbenchUpgradeDataAsset.h"
#include "ECGameplayTags.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "Save/ECSaveSubsystem.h"

AECPlayerState::AECPlayerState()
{
	ASC = CreateDefaultSubobject<UECAbilitySystemComponent>(TEXT("ASC"));

	CreateDefaultSubobject<UECHealthSet>(TEXT("HealthSet"));
	CreateDefaultSubobject<UECStaminaSet>(TEXT("StaminaSet"));
	CreateDefaultSubobject<UECMoveSpeedSet>(TEXT("MoveSpeedSet"));
	CreateDefaultSubobject<UECCombatSet>(TEXT("CombatSet"));

	StatLevels.Emplace(EPlayerStatType::Health,  1);
	StatLevels.Emplace(EPlayerStatType::Stamina, 1);
	StatLevels.Emplace(EPlayerStatType::MoveSpeed, 1);
	StatLevels.Emplace(EPlayerStatType::Combat, 1);

	WorkbenchUpgradeLevels.Emplace(EECWorkbenchUpgradeType::MaxHealth, 1);
	WorkbenchUpgradeLevels.Emplace(EECWorkbenchUpgradeType::MaxStamina, 1);
	WorkbenchUpgradeLevels.Emplace(EECWorkbenchUpgradeType::MaxInventoryWeight, 1);
	WorkbenchUpgradeLevels.Emplace(EECWorkbenchUpgradeType::Axe, 1);
	WorkbenchUpgradeLevels.Emplace(EECWorkbenchUpgradeType::Pickaxe, 1);
	WorkbenchUpgradeLevels.Emplace(EECWorkbenchUpgradeType::Weapon, 1);
}

void AECPlayerState::BeginPlay()
{
	Super::BeginPlay();

	SetCoin(StartingCoin);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UECSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UECSaveSubsystem>())
		{
			SaveSubsystem->RegisterSaveable(this);
		}
	}
	
	InitAbilities();
	InitStats();
}

void AECPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UECSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UECSaveSubsystem>())
		{
			SaveSubsystem->UnregisterSaveable(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AECPlayerState::WriteSaveData(FECGameplaySaveData& InOutSaveData) const
{
	InOutSaveData.Player.Coin = Coin;
	InOutSaveData.Player.HealthLevel = GetWorkbenchUpgradeLevel(EECWorkbenchUpgradeType::MaxHealth);
	InOutSaveData.Player.StaminaLevel = GetWorkbenchUpgradeLevel(EECWorkbenchUpgradeType::MaxStamina);
	InOutSaveData.Player.MoveSpeedLevel = FMath::Max(1, StatLevels.FindRef(EPlayerStatType::MoveSpeed));
	InOutSaveData.Player.CombatLevel = FMath::Max(1, StatLevels.FindRef(EPlayerStatType::Combat));
	InOutSaveData.Player.MaxInventoryWeightLevel = GetWorkbenchUpgradeLevel(EECWorkbenchUpgradeType::MaxInventoryWeight);
	InOutSaveData.Player.AxeLevel = GetWorkbenchUpgradeLevel(EECWorkbenchUpgradeType::Axe);
	InOutSaveData.Player.PickaxeLevel = GetWorkbenchUpgradeLevel(EECWorkbenchUpgradeType::Pickaxe);
	InOutSaveData.Player.WeaponLevel = GetWorkbenchUpgradeLevel(EECWorkbenchUpgradeType::Weapon);
}

void AECPlayerState::ReadSaveData(const FECGameplaySaveData& SaveData)
{
	SetCoin(SaveData.Player.Coin);

	CommitWorkbenchUpgradeLevel(
		EECWorkbenchUpgradeType::MaxHealth,
		NormalizeLoadedWorkbenchUpgradeLevel(EECWorkbenchUpgradeType::MaxHealth, SaveData.Player.HealthLevel));
	CommitWorkbenchUpgradeLevel(
		EECWorkbenchUpgradeType::MaxStamina,
		NormalizeLoadedWorkbenchUpgradeLevel(EECWorkbenchUpgradeType::MaxStamina, SaveData.Player.StaminaLevel));
	CommitWorkbenchUpgradeLevel(
		EECWorkbenchUpgradeType::MaxInventoryWeight,
		NormalizeLoadedWorkbenchUpgradeLevel(EECWorkbenchUpgradeType::MaxInventoryWeight, SaveData.Player.MaxInventoryWeightLevel));
	CommitWorkbenchUpgradeLevel(
		EECWorkbenchUpgradeType::Axe,
		NormalizeLoadedWorkbenchUpgradeLevel(EECWorkbenchUpgradeType::Axe, SaveData.Player.AxeLevel));
	CommitWorkbenchUpgradeLevel(
		EECWorkbenchUpgradeType::Pickaxe,
		NormalizeLoadedWorkbenchUpgradeLevel(EECWorkbenchUpgradeType::Pickaxe, SaveData.Player.PickaxeLevel));
	CommitWorkbenchUpgradeLevel(
		EECWorkbenchUpgradeType::Weapon,
		NormalizeLoadedWorkbenchUpgradeLevel(EECWorkbenchUpgradeType::Weapon, SaveData.Player.WeaponLevel));

	StatLevels.FindOrAdd(EPlayerStatType::MoveSpeed) = FMath::Max(1, SaveData.Player.MoveSpeedLevel);
	StatLevels.FindOrAdd(EPlayerStatType::Combat) = FMath::Max(1, SaveData.Player.CombatLevel);
}

void AECPlayerState::SetCoin(int32 NewCoin)
{
	const int32 ClampedCoin = FMath::Max(0, NewCoin);
	if (Coin == ClampedCoin)
	{
		return;
	}

	Coin = ClampedCoin;
	OnCoinChanged.Broadcast(Coin);
}

void AECPlayerState::AddCoin(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	const int64 NewCoin = static_cast<int64>(Coin) + Amount;
	SetCoin(static_cast<int32>(FMath::Min<int64>(NewCoin, MAX_int32)));
}

bool AECPlayerState::SpendCoin(int32 Amount)
{
	if (!CanAfford(Amount))
	{
		return false;
	}

	SetCoin(Coin - Amount);
	return true;
}

bool AECPlayerState::CanAfford(int32 Amount) const
{
	return Amount >= 0 && Coin >= Amount;
}

int32 AECPlayerState::GetWorkbenchUpgradeLevel(EECWorkbenchUpgradeType UpgradeType) const
{
	if (UpgradeType == EECWorkbenchUpgradeType::MAX)
	{
		return 1;
	}

	return FMath::Max(1, WorkbenchUpgradeLevels.FindRef(UpgradeType));
}

bool AECPlayerState::GetWorkbenchUpgradeValue(
	EECWorkbenchUpgradeType UpgradeType,
	FName ValueId,
	float& OutValue) const
{
	OutValue = 0.0f;
	if (!IsValid(WorkbenchUpgradeData) || UpgradeType == EECWorkbenchUpgradeType::MAX || ValueId.IsNone())
	{
		return false;
	}

	const FECWorkbenchUpgradeLevelData* LevelData = WorkbenchUpgradeData->FindLevelData(
		UpgradeType,
		GetWorkbenchUpgradeLevel(UpgradeType));
	if (!LevelData)
	{
		return false;
	}

	const FECWorkbenchUpgradeValue* UpgradeValue = LevelData->Values.FindByPredicate(
		[ValueId](const FECWorkbenchUpgradeValue& Candidate)
		{
			return Candidate.ValueId == ValueId;
		});
	if (!UpgradeValue)
	{
		return false;
	}

	OutValue = UpgradeValue->Value;
	return true;
}

const FECWorkbenchUpgradeLevelData* AECPlayerState::ResolveNextWorkbenchUpgradeLevel(
	EECWorkbenchUpgradeType UpgradeType) const
{
	if (!IsValid(WorkbenchUpgradeData) || UpgradeType == EECWorkbenchUpgradeType::MAX)
	{
		return nullptr;
	}

	const int32 CurrentLevel = GetWorkbenchUpgradeLevel(UpgradeType);
	if (!WorkbenchUpgradeData->FindLevelData(UpgradeType, CurrentLevel))
	{
		return nullptr;
	}

	return WorkbenchUpgradeData->FindNextLevelData(UpgradeType, CurrentLevel);
}

bool AECPlayerState::CanExecuteWorkbenchUpgrade(
	EECWorkbenchUpgradeType UpgradeType) const
{
	const FECWorkbenchUpgradeLevelData* NextLevelData = ResolveNextWorkbenchUpgradeLevel(UpgradeType);
	if (!NextLevelData || NextLevelData->RequiredItems.IsEmpty())
	{
		return false;
	}

	TArray<UECInventoryManagerComponent*> MaterialInventories;
	GetWorkbenchMaterialInventories(MaterialInventories);
	return UECInventoryManagerComponent::CanConsumeItemRequirementsFromInventories(
		MaterialInventories,
		NextLevelData->RequiredItems);
}

bool AECPlayerState::ExecuteWorkbenchUpgrade(
	EECWorkbenchUpgradeType UpgradeType)
{
	// 인벤토리 갱신 콜백에서 강화 실행이 다시 호출되는 경우만 차단합니다.
	// 강화 가능 여부 조회는 최종 재료 상태를 UI에 반영할 수 있도록 허용합니다.
	if (bWorkbenchUpgradeInProgress)
	{
		return false;
	}

	const FECWorkbenchUpgradeLevelData* NextLevelData = ResolveNextWorkbenchUpgradeLevel(UpgradeType);
	if (!NextLevelData || NextLevelData->RequiredItems.IsEmpty())
	{
		return false;
	}

	const int32 NextLevel = NextLevelData->Level;
	const TArray<FECInventoryItemRequirement> RequiredItems = NextLevelData->RequiredItems;
	TArray<UECInventoryManagerComponent*> MaterialInventories;
	GetWorkbenchMaterialInventories(MaterialInventories);

	TGuardValue<bool> UpgradeGuard(bWorkbenchUpgradeInProgress, true);
	return UECInventoryManagerComponent::ConsumeItemRequirementsFromInventoriesWithCommit(
		MaterialInventories,
		RequiredItems,
		[this, UpgradeType, NextLevel]()
		{
			CommitWorkbenchUpgradeLevel(UpgradeType, NextLevel);
		});
}

int32 AECPlayerState::GetWorkbenchMaterialCount(
	TSubclassOf<UECInventoryItemDefinition> ItemDefinition) const
{
	if (!ItemDefinition)
	{
		return 0;
	}

	TArray<UECInventoryManagerComponent*> MaterialInventories;
	GetWorkbenchMaterialInventories(MaterialInventories);

	int64 TotalCount = 0;
	for (const UECInventoryManagerComponent* Inventory : MaterialInventories)
	{
		TotalCount += Inventory->GetItemCountByDefinition(ItemDefinition);
		if (TotalCount >= MAX_int32)
		{
			return MAX_int32;
		}
	}

	return static_cast<int32>(TotalCount);
}

void AECPlayerState::GetWorkbenchMaterialInventories(
	TArray<UECInventoryManagerComponent*>& OutInventories) const
{
	OutInventories.Reset();

	UECInventoryManagerComponent* PlayerInventory = ResolvePlayerInventory();
	if (!IsValid(PlayerInventory))
	{
		return;
	}

	// 재료는 가방에서 먼저 차감하고, 부족한 수량만 보관함에서 이어서 차감합니다.
	OutInventories.Add(PlayerInventory);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AECInventoryContainer> It(World); It; ++It)
	{
		AECInventoryContainer* Container = *It;
		if (!IsValid(Container)
			|| Container->InventoryUITag != ECGameplayTags::InputTag_UI_Inventory_Storage)
		{
			continue;
		}

		if (UECInventoryManagerComponent* StorageInventory = Container->GetInventoryManagerComponent())
		{
			OutInventories.AddUnique(StorageInventory);
		}
	}
}

UECInventoryManagerComponent* AECPlayerState::ResolvePlayerInventory() const
{
	const AECPlayerController* PlayerController = Cast<AECPlayerController>(GetPlayerController());
	return PlayerController ? PlayerController->GetInventoryManagerComponent() : nullptr;
}

int32 AECPlayerState::NormalizeLoadedWorkbenchUpgradeLevel(
	EECWorkbenchUpgradeType UpgradeType,
	int32 SavedLevel) const
{
	const int32 LowerBoundedLevel = FMath::Max(1, SavedLevel);
	if (!IsValid(WorkbenchUpgradeData))
	{
		UE_LOG(LogTemp, Error,
			TEXT("WorkbenchUpgradeData가 설정되지 않아 %s 저장 단계를 1로 초기화합니다. 저장값: %d"),
			*StaticEnum<EECWorkbenchUpgradeType>()->GetNameStringByValue(static_cast<int64>(UpgradeType)),
			SavedLevel);
		return 1;
	}

	const int32 MaxLevel = WorkbenchUpgradeData->GetMaxConfiguredLevel(UpgradeType);
	if (MaxLevel <= 0)
	{
		UE_LOG(LogTemp, Error,
			TEXT("WorkbenchUpgradeData가 유효하지 않아 %s 저장 단계를 1로 초기화합니다."),
			*StaticEnum<EECWorkbenchUpgradeType>()->GetNameStringByValue(static_cast<int64>(UpgradeType)));
		return 1;
	}

	return FMath::Clamp(LowerBoundedLevel, 1, MaxLevel);
}

void AECPlayerState::CommitWorkbenchUpgradeLevel(
	EECWorkbenchUpgradeType UpgradeType,
	int32 NewLevel)
{
	check(UpgradeType != EECWorkbenchUpgradeType::MAX);
	check(NewLevel >= 1);

	int32& CurrentLevel = WorkbenchUpgradeLevels.FindOrAdd(UpgradeType, 1);
	const bool bLevelChanged = CurrentLevel != NewLevel;
	CurrentLevel = NewLevel;

	// 기존 Health/Stamina 레벨 저장소와 진행 상태를 맞춘다. 실제 Attribute 적용은 이후 단계에서 수행한다.
	if (UpgradeType == EECWorkbenchUpgradeType::MaxHealth)
	{
		StatLevels.FindOrAdd(EPlayerStatType::Health) = NewLevel;
	}
	else if (UpgradeType == EECWorkbenchUpgradeType::MaxStamina)
	{
		StatLevels.FindOrAdd(EPlayerStatType::Stamina) = NewLevel;
	}

	ApplyWorkbenchUpgradeEffect(UpgradeType);

	if (bLevelChanged)
	{
		OnWorkbenchUpgradeLevelChanged.Broadcast(UpgradeType, NewLevel);
	}
}

void AECPlayerState::ApplyWorkbenchUpgradeEffect(EECWorkbenchUpgradeType UpgradeType)
{
	if (UpgradeType == EECWorkbenchUpgradeType::Axe || UpgradeType == EECWorkbenchUpgradeType::Pickaxe)
	{
		// 도구 단계에 따른 유효 무게는 인벤토리 조회 시 계산됩니다.
		// 강화 재료 트랜잭션의 최종 인벤토리 알림이 무게 HUD도 갱신합니다.
		return;
	}

	const bool bSupportedStat =
		UpgradeType == EECWorkbenchUpgradeType::MaxHealth ||
		UpgradeType == EECWorkbenchUpgradeType::MaxStamina ||
		UpgradeType == EECWorkbenchUpgradeType::MaxInventoryWeight;
	if (!bSupportedStat || !IsValid(WorkbenchUpgradeData))
	{
		return;
	}
	if (UpgradeType != EECWorkbenchUpgradeType::MaxInventoryWeight && !ASC)
	{
		return;
	}

	const FECWorkbenchUpgradeLevelData* LevelData = WorkbenchUpgradeData->FindLevelData(
		UpgradeType,
		GetWorkbenchUpgradeLevel(UpgradeType));
	if (!LevelData)
	{
		return;
	}

	FName ValueId;
	switch (UpgradeType)
	{
	case EECWorkbenchUpgradeType::MaxHealth:
		ValueId = TEXT("MaxHealth");
		break;
	case EECWorkbenchUpgradeType::MaxStamina:
		ValueId = TEXT("MaxStamina");
		break;
	case EECWorkbenchUpgradeType::MaxInventoryWeight:
		ValueId = TEXT("MaxInventoryWeight");
		break;
	default:
		return;
	}
	const FECWorkbenchUpgradeValue* StatValue = LevelData->Values.FindByPredicate(
		[ValueId](const FECWorkbenchUpgradeValue& UpgradeValue)
		{
			return UpgradeValue.ValueId == ValueId;
		});

	if (!StatValue || StatValue->Value <= 0.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("%s 강화 단계에 유효한 %s 값이 없습니다."),
			*StaticEnum<EECWorkbenchUpgradeType>()->GetNameStringByValue(static_cast<int64>(UpgradeType)),
			*ValueId.ToString());
		return;
	}

	if (UpgradeType == EECWorkbenchUpgradeType::MaxHealth)
	{
		ASC->SetNumericAttributeBase(UECHealthSet::GetMaxHealthAttribute(), StatValue->Value);
		ASC->SetNumericAttributeBase(UECHealthSet::GetHealthAttribute(), StatValue->Value);
	}
	else if (UpgradeType == EECWorkbenchUpgradeType::MaxStamina)
	{
		ASC->SetNumericAttributeBase(UECStaminaSet::GetMaxStaminaAttribute(), StatValue->Value);
		ASC->SetNumericAttributeBase(UECStaminaSet::GetStaminaAttribute(), StatValue->Value);
	}
	else if (UECInventoryManagerComponent* Inventory = ResolvePlayerInventory())
	{
		Inventory->SetMaxInventoryWeight(StatValue->Value);
	}
}

void AECPlayerState::InitStats()
{
	if (!ASC) return;

#if WITH_EDITOR
	// 에디터에서 수정한 Attribute 기본값을 반영하도록 GAS 캐시를 다시 생성합니다.
	CastChecked<UECAbilitySystemGlobals>(IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals())->ReloadAttributeDefaultsForEditor();
#endif

	// bInitialInit = true → 최초 1회. 이후 LevelUp 시에는 false로 호출됩니다.
	for (auto& [StatType, Level] : StatLevels)
	{
		ApplyStatInit(StatType, Level, true);
	}

	// Curve 초기화 이후 작업대 데이터의 현재 강화 단계를 최종 적용합니다.
	ApplyWorkbenchUpgradeEffect(EECWorkbenchUpgradeType::MaxHealth);
	ApplyWorkbenchUpgradeEffect(EECWorkbenchUpgradeType::MaxStamina);
	ApplyWorkbenchUpgradeEffect(EECWorkbenchUpgradeType::MaxInventoryWeight);
	bIsStatsInitialized = true;
	// 초기화가 완료된 후, HUD 초기화를 위해 이벤트를 전달합니다.(PlayerState -> PlayerController)
	OnStatsInitializedDelegate.Broadcast();
}

void AECPlayerState::ApplyStatInit(EPlayerStatType StatType, int32 Level, bool bInitialInit)
{
	if (!ASC) return;
	
	UAbilitySystemGlobals* ASG = IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals();
	if (!ASG) return;

	FAttributeSetInitter* Initter = ASG->GetAttributeSetInitter();
	if (!Initter) return;

	// Enum을 FName으로 변환합니다.
	FName StatGroupName = FName(StaticEnum<EPlayerStatType>()->GetNameStringByValue((int32)StatType));
	
	// Global Data Table에서 Group을 찾아 Level에 맞게 값을 수정합니다.
	Initter->InitAttributeSetDefaults(ASC, StatGroupName, Level, bInitialInit);

	// Max 값이 갱신되었으니 현재 값도 Max에 맞춥니다. (초기화/레벨업 모두 풀회복)
	SyncCurrentToMax(StatType);

	UE_LOG(LogTemp, Warning, TEXT(" 스탯 갱신 "));
}

void AECPlayerState::SyncCurrentToMax(EPlayerStatType StatType)
{
	if (!ASC) return;

	switch (StatType)
	{
		case EPlayerStatType::Health:
			if (const UECHealthSet* HealthSet = ASC->GetSet<UECHealthSet>())
			{
				ASC->SetNumericAttributeBase(UECHealthSet::GetHealthAttribute(), HealthSet->GetMaxHealth());
			}
			break;

		case EPlayerStatType::Stamina:
			if (const UECStaminaSet* StaminaSet = ASC->GetSet<UECStaminaSet>())
			{
				ASC->SetNumericAttributeBase(UECStaminaSet::GetStaminaAttribute(), StaminaSet->GetMaxStamina());
			}
			break;

		default:
			break;
	}
}

void AECPlayerState::LevelUpStat(EPlayerStatType StatType, int32 InLevel)
{
	if (StatType == EPlayerStatType::Health || StatType == EPlayerStatType::Stamina)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Health/Stamina 강화는 ExecuteWorkbenchUpgrade만 사용할 수 있습니다: %s"),
			*StaticEnum<EPlayerStatType>()->GetNameStringByValue(static_cast<int64>(StatType)));
		return;
	}

	if (InLevel <= 0)
	{
		return;
	}

	int32* Level = StatLevels.Find(StatType);
	if (!Level) return;
	if (*Level > MAX_int32 - InLevel)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("스탯 레벨 증가가 int32 범위를 초과해 취소되었습니다: %s"),
			*StaticEnum<EPlayerStatType>()->GetNameStringByValue(static_cast<int64>(StatType)));
		return;
	}

	(*Level) += InLevel;
	ApplyStatInit(StatType, *Level, false);
}

void AECPlayerState::InitAbilities()
{
	for (UECAbilitySet* AbilitySet : DefaultAbilitySets)
	{
		if (AbilitySet)
		{
			// OutGrantedHandle을 nullptr로 지정한다는 것은 영구 부여한다는 의미입니다.
			AbilitySet->GiveToAbilitySystem(ASC, nullptr);
		}
	}
}
