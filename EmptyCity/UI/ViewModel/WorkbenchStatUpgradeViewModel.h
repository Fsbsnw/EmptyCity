#pragma once

#include "CoreMinimal.h"
#include "Data/Workbench/ECWorkbenchUpgradeTypes.h"
#include "Inventory/ECInventoryItemRequirement.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "WorkbenchStatUpgradeViewModel.generated.h"

class AECPlayerState;
class UECInventoryManagerComponent;

DECLARE_MULTICAST_DELEGATE(FOnWorkbenchStatPreviewRefreshed);

/** 작업대 스탯 카드 하나의 현재/다음 단계 표시 데이터를 제공합니다. */
UCLASS()
class EMPTYCITY_API UWorkbenchStatUpgradeViewModel : public UECViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(AActor* ContextActor, EECWorkbenchUpgradeType InUpgradeType);
	void SetUpgradeType(EECWorkbenchUpgradeType InUpgradeType);
	void Refresh();
	bool ExecuteUpgrade();
	const TArray<FECInventoryItemRequirement>& GetNextRequirements() const { return NextRequirements; }

	virtual void BroadcastInitialValues() override;
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Stat Upgrade")
	EECWorkbenchUpgradeType GetUpgradeType() const { return UpgradeType; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Stat Upgrade")
	FText GetCurrentValueText() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Stat Upgrade")
	FText GetNextValueText() const;

	FOnWorkbenchStatPreviewRefreshed OnPreviewRefreshed;

private:
	void HandleUpgradeLevelChanged(EECWorkbenchUpgradeType ChangedType, int32 NewLevel);
	void HandleInventoryUpdated();
	void SetDisplayName(const FText& NewDisplayName);
	void SetCurrentLevel(int32 NewCurrentLevel);
	void SetMaxLevel(int32 NewMaxLevel);
	void SetCurrentValue(float NewCurrentValue);
	void SetNextValue(float NewNextValue);
	void SetHasNextLevel(bool bNewHasNextLevel);
	void SetHasValidData(bool bNewHasValidData);
	void SetCanUpgrade(bool bNewCanUpgrade);
	FName GetPrimaryValueId() const;

private:
	UPROPERTY(Transient)
	EECWorkbenchUpgradeType UpgradeType = EECWorkbenchUpgradeType::MaxHealth;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 CurrentLevel = 1;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 MaxLevel = 0;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	float CurrentValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	float NextValue = 0.0f;

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
	float GetCurrentValue() const { return CurrentValue; }
	float GetNextValue() const { return NextValue; }
	bool HasNextLevel() const { return bHasNextLevel; }
	bool HasValidData() const { return bHasValidData; }
	bool CanUpgrade() const { return bCanUpgrade; }
};
