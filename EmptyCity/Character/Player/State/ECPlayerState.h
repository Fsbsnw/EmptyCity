#pragma once

#include "CoreMinimal.h"
#include "Data/Workbench/ECWorkbenchUpgradeTypes.h"
#include "GameFramework/PlayerState.h"
#include "Save/ECSaveable.h"
#include "ECPlayerState.generated.h"

class UGameplayEffect;
class UECAbilitySystemComponent;
class UECAbilitySet;
class UECInventoryItemDefinition;
class UECInventoryManagerComponent;
class UECWorkbenchUpgradeDataAsset;
struct FECWorkbenchUpgradeLevelData;

/** 컨트롤러의 HUD 초기화 타이밍(PlayerStat 초기값 설정)을 전달하는 델리게이트입니다. */
DECLARE_MULTICAST_DELEGATE(FOnStatsInitializedDelegate);

/** 플레이어의 Coin 보유량이 변경될 때 호출됩니다. */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCoinChangedDelegate, int32);

/** 작업대 강화 대상의 저장 단계가 변경될 때 호출됩니다. */
DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnWorkbenchUpgradeLevelChangedDelegate,
	EECWorkbenchUpgradeType,
	int32);

UENUM(BlueprintType)
enum class EPlayerStatType : uint8
{
	Health		UMETA(DisplayName = "체력"),
	Stamina		UMETA(DisplayName = "스태미나"),
	MoveSpeed	UMETA(DisplayName = "이동속도"),
	Combat		UMETA(DisplayName = "공격력"),
	MAX			UMETA(Hidden)
};

UCLASS()
class EMPTYCITY_API AECPlayerState : public APlayerState, public IECSaveable
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// PlayerState Interface
// ─────────────────────────────────────────────────────────────	
public:
	AECPlayerState();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


// ─────────────────────────────────────────────────────────────
// Saveable Interface
// ─────────────────────────────────────────────────────────────
public:
	virtual void WriteSaveData(FECGameplaySaveData& InOutSaveData) const override;
	virtual void ReadSaveData(const FECGameplaySaveData& SaveData) override;


// ─────────────────────────────────────────────────────────────
// Currency Method
// ────────────────────────────────────────────────────────────
public:
	/** 현재 보유한 Coin을 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 GetCoin() const { return Coin; }

	/** Coin을 0 이상의 값으로 설정합니다. */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void SetCoin(int32 NewCoin);

	/** 교환소 판매 등으로 획득한 Coin을 추가합니다. */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void AddCoin(int32 Amount);

	/** 지불 가능하면 Coin을 차감하고 true를 반환합니다. */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool SpendCoin(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Currency")
	bool CanAfford(int32 Amount) const;

	FOnCoinChangedDelegate OnCoinChanged;


// ─────────────────────────────────────────────────────────────
// Workbench Upgrade Progression
// ─────────────────────────────────────────────────────────────
public:
	/** 스탯/도구/무기 강화의 현재 플레이어 단위 단계를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Workbench|Upgrade")
	int32 GetWorkbenchUpgradeLevel(EECWorkbenchUpgradeType UpgradeType) const;

	/** PlayerState 클래스 기본값에 지정된 작업대 강화 데이터를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Workbench|Upgrade")
	const UECWorkbenchUpgradeDataAsset* GetWorkbenchUpgradeData() const { return WorkbenchUpgradeData; }

	/** 현재 강화 단계의 이름 기반 값을 조회합니다. */
	UFUNCTION(BlueprintPure, Category = "Workbench|Upgrade")
	bool GetWorkbenchUpgradeValue(
		EECWorkbenchUpgradeType UpgradeType,
		FName ValueId,
		float& OutValue) const;

	/** 현재 단계의 다음 비용을 이 PlayerState 소유 플레이어 인벤토리로 지불할 수 있는지 검사합니다. */
	UFUNCTION(BlueprintPure, Category = "Workbench|Upgrade")
	bool CanExecuteWorkbenchUpgrade(EECWorkbenchUpgradeType UpgradeType) const;

	/** 재료를 원자적으로 소모한 뒤 정확한 다음 단계로 진행하는 유일한 공개 실행 API입니다. */
	UFUNCTION(BlueprintCallable, Category = "Workbench|Upgrade")
	bool ExecuteWorkbenchUpgrade(EECWorkbenchUpgradeType UpgradeType);

	/** 플레이어 인벤토리와 현재 레벨의 보관함을 합산한 재료 수량을 반환합니다. */
	int32 GetWorkbenchMaterialCount(TSubclassOf<UECInventoryItemDefinition> ItemDefinition) const;

	/** 강화 재료로 사용할 인벤토리를 플레이어, 보관함 순서로 반환합니다. */
	void GetWorkbenchMaterialInventories(
		TArray<UECInventoryManagerComponent*>& OutInventories) const;

	FOnWorkbenchUpgradeLevelChangedDelegate OnWorkbenchUpgradeLevelChanged;
	

// ─────────────────────────────────────────────────────────────
// Stat Method
// ─────────────────────────────────────────────────────────────
public:
	/** 플레이어의 모든 스탯을 레벨에 따라 초기화하는 함수입니다. */
	void InitStats();

	/** 해당 스탯 레벨을 InLevel만큼 올리고 어트리뷰트를 갱신합니다. */
	void LevelUpStat(EPlayerStatType StatType, int32 InLevel = 1);

	/** 플레이어의 스탯 초기화 여부를 확인하는 Getter 함수입니다. */
	FORCEINLINE bool IsStatsInitialized() const { return bIsStatsInitialized; }
	
	/** 플레이어의 스탯 초기화 작업이 완료된 후에 호출되는 델리게이트입니다. */
	FOnStatsInitializedDelegate OnStatsInitializedDelegate;

private:
	/** 어트리뷰트 그룹 하나를 지정된 레벨로 초기화합니다. */
	void ApplyStatInit(EPlayerStatType StatType, int32 Level, bool bInitialInit);

	/** 해당 스탯의 현재 값을 Max 값에 맞춰 동기화합니다. (초기화/레벨업 시 풀회복 용도) */
	void SyncCurrentToMax(EPlayerStatType StatType);

	/** 플레이어의 스탯 초기화 여부를 판단하는 변수입니다. */
	bool bIsStatsInitialized = false;


// ─────────────────────────────────────────────────────────────
// Ability Method
// ─────────────────────────────────────────────────────────────
private:
	/** 플레이어가 기본으로 사용하는 어빌리티를 등록하는 함수입니다. */
	void InitAbilities();
	
	
// ─────────────────────────────────────────────────────────────
// Component
// ─────────────────────────────────────────────────────────────		
protected:
	/** 플레이어의 AbilitySystemComponent입니다. OnPossess 호출 시점에 동기화됩니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UECAbilitySystemComponent> ASC;
	
	
// ─────────────────────────────────────────────────────────────
// Ability Variable
// ─────────────────────────────────────────────────────────────
private:
	/** 플레이어가 기본으로 사용하는 어빌리티에 대한 정보입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|어빌리티")
	TArray<TObjectPtr<UECAbilitySet>> DefaultAbilitySets;


// ─────────────────────────────────────────────────────────────
// Stat Variable
// ─────────────────────────────────────────────────────────────
private:
	UPROPERTY()
	TMap<EPlayerStatType, int32> StatLevels;

	/** 작업대 강화의 플레이어 전역 진행 상태입니다. 모든 대상은 1단계에서 시작합니다. */
	UPROPERTY(VisibleInstanceOnly, SaveGame, Category = "Workbench|Upgrade")
	TMap<EECWorkbenchUpgradeType, int32> WorkbenchUpgradeLevels;

	/** 여섯 타입의 단계/비용 정의입니다. PlayerState 클래스 기본값에서 지정합니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Workbench|Upgrade")
	TObjectPtr<UECWorkbenchUpgradeDataAsset> WorkbenchUpgradeData;

	const FECWorkbenchUpgradeLevelData* ResolveNextWorkbenchUpgradeLevel(
		EECWorkbenchUpgradeType UpgradeType) const;
	UECInventoryManagerComponent* ResolvePlayerInventory() const;
	int32 NormalizeLoadedWorkbenchUpgradeLevel(EECWorkbenchUpgradeType UpgradeType, int32 SavedLevel) const;
	void CommitWorkbenchUpgradeLevel(EECWorkbenchUpgradeType UpgradeType, int32 NewLevel);
	void ApplyWorkbenchUpgradeEffect(EECWorkbenchUpgradeType UpgradeType);

	/** 인벤토리 갱신 콜백에서 동일 강화를 재진입하지 못하게 합니다. */
	bool bWorkbenchUpgradeInProgress = false;


// ──────────────────────────────────────────────────────────────
// Currency Variable
// ─────────────────────────────────────────────────────────────
private:
	/** 새 게임에서 시작할 때 지급할 Coin입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Currency", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 StartingCoin = 0;

	/** 아이템으로 등록되지 않는 플레이어 전용 재화 데이터입니다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category = "Currency", meta = (AllowPrivateAccess = "true"))
	int32 Coin = 0;


// ─────────────────────────────────────────────────────────────
// Getter
// ─────────────────────────────────────────────────────────────
public:
	FORCEINLINE UECAbilitySystemComponent* GetAbilitySystemComponent() const { return ASC; }
	
};
