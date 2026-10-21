#include "UI/ViewModel/WorkbenchUpgradeRequirementViewModel.h"

#include "Character/Player/Controller/ECPlayerController.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Data/Item/ECItemTableRow.h"
#include "Engine/Texture2D.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "Subsystem/ECItemDataSubsystem.h"

void UWorkbenchUpgradeRequirementViewModel::Initialize(
	AActor* ContextActor,
	const FECInventoryItemRequirement& InRequirement)
{
	UECViewModelBase::BindCallbacksToDependencies(ContextActor);

	if (UECInventoryManagerComponent* PreviousInventory = InventoryComponent.Get())
	{
		PreviousInventory->OnInventoryUpdated.RemoveAll(this);
	}

	const AECPlayerController* PlayerController = Cast<AECPlayerController>(GetPlayerController());
	InventoryComponent = PlayerController ? PlayerController->GetInventoryManagerComponent() : nullptr;

	if (UECInventoryManagerComponent* Inventory = InventoryComponent.Get())
	{
		Inventory->OnInventoryUpdated.AddUObject(this, &ThisClass::HandleInventoryUpdated);
	}

	SetRequirement(InRequirement);
}

void UWorkbenchUpgradeRequirementViewModel::SetRequirement(
	const FECInventoryItemRequirement& InRequirement)
{
	if (ItemDefinition != InRequirement.ItemDefinition)
	{
		ItemDefinition = InRequirement.ItemDefinition;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemDefinition);
	}

	RefreshItemDisplayData();
	SetRequiredCount(FMath::Max(0, InRequirement.Count));
	RefreshFromInventory();
}

void UWorkbenchUpgradeRequirementViewModel::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemDefinition);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemDisplayName);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemIcon);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(RequiredCount);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(OwnedCount);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(bIsSatisfied);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetOwnedCountText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetRequiredCountText);
}

void UWorkbenchUpgradeRequirementViewModel::BeginDestroy()
{
	if (UECInventoryManagerComponent* Inventory = InventoryComponent.Get())
	{
		Inventory->OnInventoryUpdated.RemoveAll(this);
	}

	InventoryComponent.Reset();
	Super::BeginDestroy();
}

FText UWorkbenchUpgradeRequirementViewModel::GetOwnedCountText() const
{
	return FText::AsNumber(OwnedCount);
}

FText UWorkbenchUpgradeRequirementViewModel::GetRequiredCountText() const
{
	return FText::AsNumber(RequiredCount);
}

void UWorkbenchUpgradeRequirementViewModel::HandleInventoryUpdated()
{
	RefreshFromInventory();
}

void UWorkbenchUpgradeRequirementViewModel::RefreshItemDisplayData()
{
	FText NewDisplayName;
	UTexture2D* NewItemIcon = nullptr;

	if (ItemDefinition && GetWorld())
	{
		const FECItemTableRow* ItemData = UECItemDataSubsystem::Get(this).FindItemData(ItemDefinition);
		if (ItemData)
		{
			NewDisplayName = ItemData->DisplayName;
			NewItemIcon = ItemData->Icon.LoadSynchronous();
		}
	}

	if (!ItemDisplayName.EqualTo(NewDisplayName))
	{
		ItemDisplayName = MoveTemp(NewDisplayName);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemDisplayName);
	}

	if (ItemIcon != NewItemIcon)
	{
		ItemIcon = NewItemIcon;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemIcon);
	}
}

void UWorkbenchUpgradeRequirementViewModel::RefreshFromInventory()
{
	const AECPlayerState* CurrentPlayerState = Cast<AECPlayerState>(GetPlayerState());
	SetOwnedCount(CurrentPlayerState && ItemDefinition
		? CurrentPlayerState->GetWorkbenchMaterialCount(ItemDefinition)
		: 0);
	SetIsSatisfied(ItemDefinition && RequiredCount > 0 && OwnedCount >= RequiredCount);
}

void UWorkbenchUpgradeRequirementViewModel::SetRequiredCount(int32 NewRequiredCount)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(RequiredCount, NewRequiredCount))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetRequiredCountText);
	}
}

void UWorkbenchUpgradeRequirementViewModel::SetOwnedCount(int32 NewOwnedCount)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(OwnedCount, FMath::Max(0, NewOwnedCount)))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetOwnedCountText);
	}
}

void UWorkbenchUpgradeRequirementViewModel::SetIsSatisfied(bool bNewIsSatisfied)
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsSatisfied, bNewIsSatisfied);
}
