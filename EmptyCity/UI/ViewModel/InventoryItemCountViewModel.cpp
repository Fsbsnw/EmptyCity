#include "UI/ViewModel/InventoryItemCountViewModel.h"

#include "Character/Player/Controller/ECPlayerController.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "Inventory/ECInventoryManagerComponent.h"

void UInventoryItemCountViewModel::Initialize(
	AActor* ContextActor,
	TSubclassOf<UECInventoryItemDefinition> InItemDefinition)
{
	UECViewModelBase::BindCallbacksToDependencies(ContextActor);

	if (UECInventoryManagerComponent* PreviousInventory = InventoryComponent.Get())
	{
		PreviousInventory->OnInventoryUpdated.RemoveAll(this);
	}

	ItemDefinition = InItemDefinition;

	AECPlayerController* PlayerController = Cast<AECPlayerController>(GetPlayerController());
	InventoryComponent = PlayerController ? PlayerController->GetInventoryManagerComponent() : nullptr;

	if (UECInventoryManagerComponent* Inventory = InventoryComponent.Get())
	{
		Inventory->OnInventoryUpdated.AddUObject(this, &ThisClass::HandleInventoryUpdated);
	}

	RefreshItemCount();
}

void UInventoryItemCountViewModel::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ItemCount);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemCountText);
}

void UInventoryItemCountViewModel::BeginDestroy()
{
	if (UECInventoryManagerComponent* Inventory = InventoryComponent.Get())
	{
		Inventory->OnInventoryUpdated.RemoveAll(this);
	}

	InventoryComponent.Reset();
	Super::BeginDestroy();
}

void UInventoryItemCountViewModel::SetItemCount(const int32 NewItemCount)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(ItemCount, FMath::Max(0, NewItemCount)))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemCountText);
	}
}

FText UInventoryItemCountViewModel::GetItemCountText() const
{
	return FText::AsNumber(ItemCount);
}

void UInventoryItemCountViewModel::SetItemDefinition(
	TSubclassOf<UECInventoryItemDefinition> NewItemDefinition)
{
	if (ItemDefinition == NewItemDefinition)
	{
		return;
	}

	ItemDefinition = NewItemDefinition;
	RefreshItemCount();
}

void UInventoryItemCountViewModel::HandleInventoryUpdated()
{
	RefreshItemCount();
}

void UInventoryItemCountViewModel::RefreshItemCount()
{
	const UECInventoryManagerComponent* Inventory = InventoryComponent.Get();
	SetItemCount(Inventory && ItemDefinition
		? Inventory->GetItemCountByDefinition(ItemDefinition)
		: 0);
}
