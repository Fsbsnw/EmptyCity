#pragma once

#include "CoreMinimal.h"
#include "Data/Workbench/ECWorkbenchUpgradeTypes.h"
#include "Inventory/ECInventoryItemRequirement.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "WorkbenchToolUpgradeViewModel.generated.h"

class AECPlayerState;
class UECInventoryManagerComponent;

DECLARE_MULTICAST_DELEGATE(FOnWorkbenchToolPreviewRefreshed);

/** Axe/Pickaxe 선택과 단계별 무게 미리보기 및 강화 가능 상태를 제공합니다. */
UCLASS()
class EMPTYCITY_API UWorkbenchToolUpgradeViewModel : public UECViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(AActor* ContextActor, EECWorkbenchUpgradeType InSelectedTool);
	void SetSelectedTool(EECWorkbenchUpgradeType InSelectedTool);
	void Refresh();
	bool ExecuteUpgrade();
	const TArray<FECInventoryItemRequirement>& GetNextRequirements() const { return NextRequirements; }

	virtual void BroadcastInitialValues() override;
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Tool Upgrade")
	EECWorkbenchUpgradeType GetSelectedTool() const { return SelectedTool; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Tool Upgrade")
	FText GetCurrentWeightText() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Tool Upgrade")
	FText GetNextWeightText() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Tool Upgrade")
	FText GetCurrentInteractionDurationText() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Tool Upgrade")
	FText GetNextInteractionDurationText() const;

	FOnWorkbenchToolPreviewRefreshed OnPreviewRefreshed;

private:
	void HandleUpgradeLevelChanged(EECWorkbenchUpgradeType ChangedType, int32 NewLevel);
	void HandleInventoryUpdated();
	void SetDisplayName(const FText& NewDisplayName);
	void SetCurrentLevel(int32 NewCurrentLevel);
	void SetMaxLevel(int32 NewMaxLevel);
	void SetCurrentWeight(float NewCurrentWeight);
	void SetNextWeight(float NewNextWeight);
	void SetCurrentInteractionDuration(float NewDuration);
	void SetNextInteractionDuration(float NewDuration);
	void SetHasNextLevel(bool bNewHasNextLevel);
	void SetHasValidData(bool bNewHasValidData);
	void SetCanUpgrade(bool bNewCanUpgrade);

	UPROPERTY(Transient)
	EECWorkbenchUpgradeType SelectedTool = EECWorkbenchUpgradeType::Axe;

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
	float CurrentInteractionDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	float NextInteractionDuration = 0.0f;

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
	float GetCurrentInteractionDuration() const { return CurrentInteractionDuration; }
	float GetNextInteractionDuration() const { return NextInteractionDuration; }
	bool HasNextLevel() const { return bHasNextLevel; }
	bool HasValidData() const { return bHasValidData; }
	bool CanUpgrade() const { return bCanUpgrade; }
};
