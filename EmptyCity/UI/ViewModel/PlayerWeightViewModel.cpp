#include "UI/ViewModel/PlayerWeightViewModel.h"

#include "Character/Player/Controller/ECPlayerController.h"
#include "Inventory/ECInventoryManagerComponent.h"

void UPlayerWeightViewModel::BindCallbacksToDependencies(AActor* ContextActor)
{
	Super::BindCallbacksToDependencies(ContextActor);

	if (UECInventoryManagerComponent* PreviousInventory = InventoryComponent.Get())
	{
		PreviousInventory->OnInventoryUpdated.RemoveAll(this);
		PreviousInventory->OnMaxInventoryWeightChanged.RemoveAll(this);
	}

	AECPlayerController* PlayerController = Cast<AECPlayerController>(GetPlayerController());
	InventoryComponent = PlayerController ? PlayerController->GetInventoryManagerComponent() : nullptr;

	if (UECInventoryManagerComponent* Inventory = InventoryComponent.Get())
	{
		Inventory->OnInventoryUpdated.AddUObject(this, &ThisClass::HandleInventoryUpdated);
		Inventory->OnMaxInventoryWeightChanged.AddUObject(
			this,
			&ThisClass::HandleMaxInventoryWeightChanged);
	}

	RefreshWeight();
}

void UPlayerWeightViewModel::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentWeight);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(MaxWeight);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetWeightPercent);
}

void UPlayerWeightViewModel::BeginDestroy()
{
	if (UECInventoryManagerComponent* Inventory = InventoryComponent.Get())
	{
		Inventory->OnInventoryUpdated.RemoveAll(this);
		Inventory->OnMaxInventoryWeightChanged.RemoveAll(this);
	}

	InventoryComponent.Reset();

	Super::BeginDestroy();
}

void UPlayerWeightViewModel::SetCurrentWeight(float NewWeight)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(CurrentWeight, FMath::Max(0.0f, NewWeight)))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetWeightPercent);
	}
}

void UPlayerWeightViewModel::SetMaxWeight(float NewWeight)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxWeight, FMath::Max(0.0f, NewWeight)))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetWeightPercent);
	}
}

float UPlayerWeightViewModel::GetWeightPercent() const
{
	if (MaxWeight <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentWeight / MaxWeight, 0.0f, 1.0f);
}

void UPlayerWeightViewModel::HandleInventoryUpdated()
{
	RefreshWeight();
}

void UPlayerWeightViewModel::HandleMaxInventoryWeightChanged(float NewMaxInventoryWeight)
{
	SetMaxWeight(NewMaxInventoryWeight);
}

void UPlayerWeightViewModel::RefreshWeight()
{
	const UECInventoryManagerComponent* Inventory = InventoryComponent.Get();
	SetCurrentWeight(Inventory ? Inventory->GetTotalWeight() : 0.0f);
	SetMaxWeight(Inventory ? Inventory->GetMaxInventoryWeight() : 0.0f);
}
