#include "UI/ViewModel/QuickBarViewModel.h"

#include "Character/Player/Controller/ECPlayerController.h"
#include "Data/Item/ECItemTableRow.h"
#include "Equipment/ECQuickBarComponent.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "Subsystem/ECItemDataSubsystem.h"

void UQuickBarViewModel::BindCallbacksToDependencies(AActor* ContextActor)
{
	Super::BindCallbacksToDependencies(ContextActor);

	if (UECQuickBarComponent* PreviousQuickBar = QuickBarComponent.Get())
	{
		PreviousQuickBar->OnQuickBarSlotChanged.RemoveAll(this);
		PreviousQuickBar->OnQuickBarSlotUsed.RemoveAll(this);
	}

	if (UECInventoryManagerComponent* PreviousInventory = InventoryComponent.Get())
	{
		PreviousInventory->OnInventoryUpdated.RemoveAll(this);
	}

	AECPlayerController* PlayerController = Cast<AECPlayerController>(GetPlayerController());
	QuickBarComponent = PlayerController ? PlayerController->GetQuickBarComponent() : nullptr;
	InventoryComponent = PlayerController ? PlayerController->GetInventoryManagerComponent() : nullptr;

	if (UECQuickBarComponent* QuickBar = QuickBarComponent.Get())
	{
		QuickBar->OnQuickBarSlotChanged.AddUObject(this, &ThisClass::HandleQuickBarSlotChanged);
		QuickBar->OnQuickBarSlotUsed.AddUObject(this, &ThisClass::HandleQuickBarSlotUsed);
	}

	if (UECInventoryManagerComponent* Inventory = InventoryComponent.Get())
	{
		Inventory->OnInventoryUpdated.AddUObject(this, &ThisClass::HandleInventoryUpdated);
	}

	RefreshAllSlots();
}

void UQuickBarViewModel::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Slot1Data);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Slot2Data);
}

void UQuickBarViewModel::BeginDestroy()
{
	if (UECQuickBarComponent* QuickBar = QuickBarComponent.Get())
	{
		QuickBar->OnQuickBarSlotChanged.RemoveAll(this);
		QuickBar->OnQuickBarSlotUsed.RemoveAll(this);
	}

	if (UECInventoryManagerComponent* Inventory = InventoryComponent.Get())
	{
		Inventory->OnInventoryUpdated.RemoveAll(this);
	}

	QuickBarComponent.Reset();
	InventoryComponent.Reset();

	Super::BeginDestroy();
}

void UQuickBarViewModel::SetSlot1Data(const FECQuickSlotViewData& NewSlotData)
{
	UE_MVVM_SET_PROPERTY_VALUE(Slot1Data, NewSlotData);
}

void UQuickBarViewModel::SetSlot2Data(const FECQuickSlotViewData& NewSlotData)
{
	UE_MVVM_SET_PROPERTY_VALUE(Slot2Data, NewSlotData);
}

void UQuickBarViewModel::HandleQuickBarSlotChanged(int32 SlotIndex, UECInventoryItemInstance* ItemInstance)
{
	RefreshSlot(SlotIndex);
}

void UQuickBarViewModel::HandleQuickBarSlotUsed(int32 SlotIndex)
{
	// 새 슬롯과 이전 슬롯의 강조 상태가 함께 바뀝니다.
	RefreshAllSlots();
}

void UQuickBarViewModel::HandleInventoryUpdated()
{
	RefreshAllSlots();
}

void UQuickBarViewModel::RefreshAllSlots()
{
	RefreshSlot(0);
	RefreshSlot(1);
}

void UQuickBarViewModel::RefreshSlot(int32 SlotIndex)
{
	if (SlotIndex == 0)
	{
		SetSlot1Data(MakeSlotViewData(SlotIndex));
	}
	else if (SlotIndex == 1)
	{
		SetSlot2Data(MakeSlotViewData(SlotIndex));
	}
}

FECQuickSlotViewData UQuickBarViewModel::MakeSlotViewData(int32 SlotIndex) const
{
	FECQuickSlotViewData Result;

	const UECQuickBarComponent* QuickBar = QuickBarComponent.Get();
	if (!QuickBar)
	{
		return Result;
	}

	const TArray<UECInventoryItemInstance*> Slots = QuickBar->GetSlots();
	UECInventoryItemInstance* ItemInstance = Slots.IsValidIndex(SlotIndex) ? Slots[SlotIndex] : nullptr;
	if (!IsValid(ItemInstance))
	{
		return Result;
	}

	Result.bAssigned = true;

	if (const FECItemTableRow* ItemData = UECItemDataSubsystem::Get(this).FindItemData(ItemInstance))
	{
		Result.ItemIcon = ItemData->Icon;
	}

	if (const UECInventoryManagerComponent* Inventory = InventoryComponent.Get())
	{
		Result.StockCount = Inventory->GetItemCountByDefinition(ItemInstance->ItemDef);
	}

	Result.bCanUse = Result.StockCount > 0;
	Result.bIsLastUsed = QuickBar->GetLastUsedSlotIndex() == SlotIndex;
	return Result;
}
