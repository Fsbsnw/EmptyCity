#include "UI/Widget/Inventory/ECInventoryItemCountWidget.h"

#include "UI/ViewModel/InventoryItemCountViewModel.h"

void UECInventoryItemCountWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ItemCountViewModel)
	{
		ItemCountViewModel = NewObject<UInventoryItemCountViewModel>(this);
	}

	InjectViewModel(ItemCountViewModel);
	ItemCountViewModel->Initialize(GetOwningPlayer(), ItemDefinition);
	ItemCountViewModel->BroadcastInitialValues();
	OnViewModelSet();
}

void UECInventoryItemCountWidget::SetItemDefinition(
	TSubclassOf<UECInventoryItemDefinition> NewItemDefinition)
{
	ItemDefinition = NewItemDefinition;

	if (ItemCountViewModel)
	{
		ItemCountViewModel->SetItemDefinition(ItemDefinition);
	}
}
