#pragma once

#include "CoreMinimal.h"
#include "Inventory/ECInventoryItemRequirement.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "WorkbenchUpgradeRequirementViewModel.generated.h"

class UECInventoryItemDefinition;
class UECInventoryManagerComponent;
class UTexture2D;

/** 강화 재료 한 종류의 필요 수량, 보유 수량, 충족 여부를 제공하는 ViewModel입니다. */
UCLASS()
class EMPTYCITY_API UWorkbenchUpgradeRequirementViewModel : public UECViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(AActor* ContextActor, const FECInventoryItemRequirement& InRequirement);
	void SetRequirement(const FECInventoryItemRequirement& InRequirement);

	virtual void BroadcastInitialValues() override;
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Upgrade")
	TSubclassOf<UECInventoryItemDefinition> GetItemDefinition() const { return ItemDefinition; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Upgrade")
	FText GetItemDisplayName() const { return ItemDisplayName; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Upgrade")
	UTexture2D* GetItemIcon() const { return ItemIcon; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Upgrade")
	FText GetOwnedCountText() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Workbench|Upgrade")
	FText GetRequiredCountText() const;

private:
	void HandleInventoryUpdated();
	void RefreshItemDisplayData();
	void RefreshFromInventory();
	void SetRequiredCount(int32 NewRequiredCount);
	void SetOwnedCount(int32 NewOwnedCount);
	void SetIsSatisfied(bool bNewIsSatisfied);

private:
	UPROPERTY(Transient)
	TSubclassOf<UECInventoryItemDefinition> ItemDefinition;

	UPROPERTY(Transient)
	FText ItemDisplayName;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> ItemIcon;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 RequiredCount = 0;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 OwnedCount = 0;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = "IsSatisfied", meta = (AllowPrivateAccess = "true"))
	bool bIsSatisfied = false;

	UPROPERTY(Transient)
	TWeakObjectPtr<UECInventoryManagerComponent> InventoryComponent;

public:
	int32 GetRequiredCount() const { return RequiredCount; }
	int32 GetOwnedCount() const { return OwnedCount; }
	bool IsSatisfied() const { return bIsSatisfied; }
};
