#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "UI/Widget/QuickBar/ECQuickSlotTypes.h"
#include "QuickBarViewModel.generated.h"

class UECInventoryItemInstance;
class UECInventoryManagerComponent;
class UECQuickBarComponent;

/** QuickBar와 인벤토리 상태를 HUD용 슬롯 표시 데이터로 변환하는 ViewModel입니다. */
UCLASS()
class EMPTYCITY_API UQuickBarViewModel : public UECViewModelBase
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies(AActor* ContextActor) override;
	virtual void BroadcastInitialValues() override;
	virtual void BeginDestroy() override;

	FECQuickSlotViewData GetSlot1Data() const { return Slot1Data; }
	void SetSlot1Data(const FECQuickSlotViewData& NewSlotData);

	FECQuickSlotViewData GetSlot2Data() const { return Slot2Data; }
	void SetSlot2Data(const FECQuickSlotViewData& NewSlotData);

private:
	void HandleQuickBarSlotChanged(int32 SlotIndex, UECInventoryItemInstance* ItemInstance);
	void HandleQuickBarSlotUsed(int32 SlotIndex);
	void HandleInventoryUpdated();
	void RefreshAllSlots();
	void RefreshSlot(int32 SlotIndex);
	FECQuickSlotViewData MakeSlotViewData(int32 SlotIndex) const;

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"))
	FECQuickSlotViewData Slot1Data;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"))
	FECQuickSlotViewData Slot2Data;

	UPROPERTY()
	TWeakObjectPtr<UECQuickBarComponent> QuickBarComponent;

	UPROPERTY()
	TWeakObjectPtr<UECInventoryManagerComponent> InventoryComponent;
};
