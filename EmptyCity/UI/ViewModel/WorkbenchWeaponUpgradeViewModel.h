#pragma once

#include "CoreMinimal.h"
#include "Data/Workbench/ECWorkbenchUpgradeTypes.h"
#include "Inventory/ECInventoryItemRequirement.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "WorkbenchWeaponUpgradeViewModel.generated.h"

class AECPlayerState;
class UECInventoryManagerComponent;

DECLARE_MULTICAST_DELEGATE(FOnWorkbenchWeaponPreviewRefreshed);

/** 무기 단계별 무게, 공격력, 공격 속도 미리보기와 강화 가능 상태를 제공합니다. */
UCLASS()
class EMPTYCITY_API UWorkbenchWeaponUpgradeViewModel : public UECViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(AActor* ContextActor);
	void Refresh();
	bool ExecuteUpgrade();
	const TArray<FECInventoryItemRequirement>& GetNextRequirements() const { return NextRequirements; }

	virtual void BroadcastInitialValues() override;
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Weapon Upgrade")
	FText GetCurrentWeightText() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Weapon Upgrade")
	FText GetNextWeightText() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Weapon Upgrade")
	FText GetCurrentAttackPowerText() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Weapon Upgrade")
	FText GetNextAttackPowerText() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Weapon Upgrade")
	FText GetCurrentAttackSpeedText() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Weapon Upgrade")
	FText GetNextAttackSpeedText() const;

	FOnWorkbenchWeaponPreviewRefreshed OnPreviewRefreshed;

private:
	void HandleUpgradeLevelChanged(EECWorkbenchUpgradeType ChangedType, int32 NewLevel);
	void HandleInventoryUpdated();
	void SetDisplayName(const FText& NewDisplayName);
	void SetCurrentLevel(int32 NewCurrentLevel);
	void SetMaxLevel(int32 NewMaxLevel);
	void SetCurrentWeight(float NewValue);
	void SetNextWeight(float NewValue);
	void SetCurrentAttackPower(float NewValue);
	void SetNextAttackPower(float NewValue);
	void SetCurrentAttackSpeedMultiplier(float NewValue);
	void SetNextAttackSpeedMultiplier(float NewValue);
	void SetHasNextLevel(bool bNewValue);
	void SetHasValidData(bool bNewValue);
	void SetCanUpgrade(bool bNewValue);

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 CurrentLevel = 1;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 MaxLevel = 0;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	float CurrentWeight = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	float NextWeight = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	float CurrentAttackPower = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	float NextAttackPower = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	float CurrentAttackSpeedMultiplier = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	float NextAttackSpeedMultiplier = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = "HasNextLevel", meta = (AllowPrivateAccess = "true"))
	bool bHasNextLevel = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = "HasValidData", meta = (AllowPrivateAccess = "true"))
	bool bHasValidData = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = "CanUpgrade", meta = (AllowPrivateAccess = "true"))
	bool bCanUpgrade = false;

	UPROPERTY(Transient)
	TArray<FECInventoryItemRequirement> NextRequirements;

	UPROPERTY(Transient)
	TWeakObjectPtr<AECPlayerState> PlayerState;

	UPROPERTY(Transient)
	TWeakObjectPtr<UECInventoryManagerComponent> InventoryComponent;

public:
	const FText& GetDisplayName() const { return DisplayName; }
	int32 GetCurrentLevel() const { return CurrentLevel; }
	int32 GetMaxLevel() const { return MaxLevel; }
	float GetCurrentWeight() const { return CurrentWeight; }
	float GetNextWeight() const { return NextWeight; }
	float GetCurrentAttackPower() const { return CurrentAttackPower; }
	float GetNextAttackPower() const { return NextAttackPower; }
	float GetCurrentAttackSpeedMultiplier() const { return CurrentAttackSpeedMultiplier; }
	float GetNextAttackSpeedMultiplier() const { return NextAttackSpeedMultiplier; }
	bool HasNextLevel() const { return bHasNextLevel; }
	bool HasValidData() const { return bHasValidData; }
	bool CanUpgrade() const { return bCanUpgrade; }
};
