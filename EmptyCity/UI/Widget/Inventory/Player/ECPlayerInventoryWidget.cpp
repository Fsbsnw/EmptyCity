// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/Inventory/Player/ECPlayerInventoryWidget.h"

#include "Components/Overlay.h"
#include "Components/ScrollBox.h"
#include "Components/WrapBox.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "Subsystem/ECSoundSubsystem.h"
#include "UI/ViewModel/InventoryInteractionViewModel.h"
#include "UI/ViewModel/InventoryViewModel.h"
#include "UI/Widget/Common/Item/ECItemDetailWidget.h"
#include "UI/Widget/Common/Item/ECItemQuantitySelectorWidget.h"
#include "UI/Widget/Inventory/ECInventorySlotWidget.h"

namespace InventorySoundRows
{
	const FName DiscardClick(TEXT("UI_InventorySlot_Button_Discard_Click"));
}

void UECPlayerInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	if (InventoryScrollBox_Player)
	{
		InventoryScrollBox_Player->SetOrientation(Orient_Vertical);
	}


	for (UWrapBox* CategoryWrapBox : {
		InventoryWrapBox_Player_ToolWeapon.Get(),
		InventoryWrapBox_Player_SurvivalKit.Get(),
		InventoryWrapBox_Player_Item.Get()})
	{
		if (CategoryWrapBox)
		{
			CategoryWrapBox->SetOrientation(Orient_Horizontal);
		}
	}

}

void UECPlayerInventoryWidget::NativeDestruct()
{
	UnbindInventoryViewModel();

	Super::NativeDestruct();
}

void UECPlayerInventoryWidget::OnWidgetOpened_Implementation()
{
	Super::OnWidgetOpened_Implementation();

	SetKeyboardFocus();
	ClearSelectedItem();
	RefreshInventoryUI();
}

void UECPlayerInventoryWidget::UnbindInventoryViewModel()
{
	if (CachedPlayerInventoryViewModel)
	{
		CachedPlayerInventoryViewModel->OnVMInventoryUpdated.RemoveAll(this);
	}

	CachedPlayerInventoryViewModel = nullptr;
}

FReply UECPlayerInventoryWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		ClearSelectedItem();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UECPlayerInventoryWidget::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::G)
	{
		HandleDiscardSelectedItem();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UECPlayerInventoryWidget::InjectViewModel(UMVVMViewModelBase* TargetViewModel)
{
	Super::InjectViewModel(TargetViewModel);

	UInventoryInteractionViewModel* InventoryInteractionViewModel =
		Cast<UInventoryInteractionViewModel>(TargetViewModel);
	if (!InventoryInteractionViewModel)
	{
		return;
	}

	UnbindInventoryViewModel();
	CachedPlayerInventoryViewModel = InventoryInteractionViewModel->GetPlayerInventoryViewModel();

	if (CachedPlayerInventoryViewModel)
	{
		CachedPlayerInventoryViewModel->OnVMInventoryUpdated.AddUObject(
			this,
			&ThisClass::RefreshInventoryUI);
	}
}

void UECPlayerInventoryWidget::RefreshInventoryUI()
{
	if (!SlotWidgetClass)
	{
		return;
	}

	static const TArray<FInventoryEntry> EmptyEntries;
	const TArray<FInventoryEntry> ToolWeaponEntries = CachedPlayerInventoryViewModel
		? CachedPlayerInventoryViewModel->GetInventoryEntriesByCategory(EECInventoryItemCategory::ToolWeapon)
		: EmptyEntries;
	const TArray<FInventoryEntry> SurvivalKitEntries = CachedPlayerInventoryViewModel
		? CachedPlayerInventoryViewModel->GetInventoryEntriesByCategory(EECInventoryItemCategory::SurvivalKit)
		: EmptyEntries;
	const TArray<FInventoryEntry> ItemEntries = CachedPlayerInventoryViewModel
		? CachedPlayerInventoryViewModel->GetInventoryEntriesByCategory(EECInventoryItemCategory::Item)
		: EmptyEntries;

	RefreshInventoryWrapBox(InventoryWrapBox_Player_ToolWeapon, ToolWeaponEntries);
	RefreshInventoryWrapBox(InventoryWrapBox_Player_SurvivalKit, SurvivalKitEntries);
	RefreshInventoryWrapBox(InventoryWrapBox_Player_Item, ItemEntries);

	if (IsValid(SelectedItemInstance))
	{
		if (!CachedPlayerInventoryViewModel ||
			!CachedPlayerInventoryViewModel->ContainsItem(SelectedItemInstance))
		{
			ClearSelectedItem();
		}
		else
		{
			if (ItemQuantitySelector)
			{
				ItemQuantitySelector->SetMaximumQuantity(GetSelectedItemStackCount());
			}
			if (ItemDetailWidget)
			{
				ItemDetailWidget->SetItem(SelectedItemInstance, GetSelectedItemStackCount());
				if (Overlay_ItemDetail)
				{
					Overlay_ItemDetail->SetVisibility(ESlateVisibility::Visible);
				}
			}
		}
	}

	RefreshSelectionVisuals();
}

void UECPlayerInventoryWidget::RefreshInventoryWrapBox(
	UWrapBox* TargetWrapBox,
	const TArray<FInventoryEntry>& Entries)
{
	if (!TargetWrapBox)
	{
		return;
	}

	while (TargetWrapBox->GetChildrenCount() < Entries.Num())
	{
		UECInventorySlotWidget* SlotWidget =
			CreateWidget<UECInventorySlotWidget>(this, SlotWidgetClass);
		if (!SlotWidget)
		{
			break;
		}

		SlotWidget->OnClicked.BindUObject(this, &ThisClass::HandleInventorySlotClicked);
		TargetWrapBox->AddChildToWrapBox(SlotWidget);
	}

	while (TargetWrapBox->GetChildrenCount() > Entries.Num())
	{
		TargetWrapBox->RemoveChildAt(TargetWrapBox->GetChildrenCount() - 1);
	}

	for (int32 Index = 0; Index < TargetWrapBox->GetChildrenCount(); ++Index)
	{
		UECInventorySlotWidget* SlotWidget =
			Cast<UECInventorySlotWidget>(TargetWrapBox->GetChildAt(Index));
		if (!SlotWidget)
		{
			continue;
		}

		if (Entries.IsValidIndex(Index))
		{
			SlotWidget->RefreshSlotUI(Entries[Index].Instance, Entries[Index].StackCount);
		}
		else
		{
			SlotWidget->SetEmptySlot();
		}

		const bool bIsSelected =
			IsValid(SelectedItemInstance) &&
			Entries.IsValidIndex(Index) &&
			Entries[Index].Instance == SelectedItemInstance;
		SlotWidget->SetSelectedState(bIsSelected);
	}
}

void UECPlayerInventoryWidget::RefreshSelectionVisuals()
{
	auto ApplySelection = [this](UWrapBox* TargetWrapBox, const TArray<FInventoryEntry>& Entries)
	{
		if (!TargetWrapBox)
		{
			return;
		}

		for (int32 Index = 0; Index < TargetWrapBox->GetChildrenCount(); ++Index)
		{
			UECInventorySlotWidget* SlotWidget =
				Cast<UECInventorySlotWidget>(TargetWrapBox->GetChildAt(Index));
			if (!SlotWidget)
			{
				continue;
			}

			const bool bIsSelected =
				IsValid(SelectedItemInstance) &&
				Entries.IsValidIndex(Index) &&
				Entries[Index].Instance == SelectedItemInstance;
			SlotWidget->SetSelectedState(bIsSelected);
		}
	};

	static const TArray<FInventoryEntry> EmptyEntries;
	const TArray<FInventoryEntry> ToolWeaponEntries = CachedPlayerInventoryViewModel
		? CachedPlayerInventoryViewModel->GetInventoryEntriesByCategory(EECInventoryItemCategory::ToolWeapon)
		: EmptyEntries;
	const TArray<FInventoryEntry> SurvivalKitEntries = CachedPlayerInventoryViewModel
		? CachedPlayerInventoryViewModel->GetInventoryEntriesByCategory(EECInventoryItemCategory::SurvivalKit)
		: EmptyEntries;
	const TArray<FInventoryEntry> ItemEntries = CachedPlayerInventoryViewModel
		? CachedPlayerInventoryViewModel->GetInventoryEntriesByCategory(EECInventoryItemCategory::Item)
		: EmptyEntries;

	ApplySelection(InventoryWrapBox_Player_ToolWeapon, ToolWeaponEntries);
	ApplySelection(InventoryWrapBox_Player_SurvivalKit, SurvivalKitEntries);
	ApplySelection(InventoryWrapBox_Player_Item, ItemEntries);
}

void UECPlayerInventoryWidget::HandleInventorySlotClicked(UECInventoryItemInstance* ItemInstance)
{
	if (!IsValid(ItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	SelectedItemInstance = ItemInstance;
	if (ItemQuantitySelector)
	{
		ItemQuantitySelector->ResetForSelection(GetSelectedItemStackCount());
	}
	if (ItemDetailWidget)
	{
		ItemDetailWidget->SetItem(ItemInstance, GetSelectedItemStackCount());
		if (Overlay_ItemDetail)
		{
			Overlay_ItemDetail->SetVisibility(ESlateVisibility::Visible);
		}
	}

	RefreshSelectionVisuals();
}

int32 UECPlayerInventoryWidget::GetSelectedItemStackCount() const
{
	if (!CachedPlayerInventoryViewModel || !IsValid(SelectedItemInstance))
	{
		return 0;
	}

	const FInventoryEntry* SelectedEntry =
		CachedPlayerInventoryViewModel->GetInventoryEntries().FindByPredicate(
			[this](const FInventoryEntry& Entry)
			{
				return Entry.Instance == SelectedItemInstance;
			});

	return SelectedEntry ? FMath::Max(0, SelectedEntry->StackCount) : 0;
}

int32 UECPlayerInventoryWidget::GetSelectedActionQuantity() const
{
	return ItemQuantitySelector
		? ItemQuantitySelector->GetSelectedQuantity()
		: FMath::Min(1, GetSelectedItemStackCount());
}

void UECPlayerInventoryWidget::HandleDiscardSelectedItem()
{
	if (!IsValid(SelectedItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	UInventoryInteractionViewModel* InventoryInteractionViewModel =
		GetViewModel<UInventoryInteractionViewModel>();
	if (!InventoryInteractionViewModel)
	{
		return;
	}

	const int32 RemovedCount =
		InventoryInteractionViewModel->DiscardPlayerItem(
			SelectedItemInstance,
			GetSelectedActionQuantity());
	if (RemovedCount <= 0)
	{
		return;
	}

	UECSoundSubsystem::Get(this).PlaySFXByName(InventorySoundRows::DiscardClick);

	if (!CachedPlayerInventoryViewModel ||
		!CachedPlayerInventoryViewModel->ContainsItem(SelectedItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	if (ItemDetailWidget)
	{
		ItemDetailWidget->SetItem(SelectedItemInstance, GetSelectedItemStackCount());
	}
	if (ItemQuantitySelector)
	{
		ItemQuantitySelector->SetMaximumQuantity(GetSelectedItemStackCount());
	}
	RefreshSelectionVisuals();
}

void UECPlayerInventoryWidget::ClearSelectedItem()
{
	SelectedItemInstance = nullptr;
	if (ItemQuantitySelector)
	{
		ItemQuantitySelector->ResetForSelection(0);
	}
	RefreshSelectionVisuals();

	if (ItemDetailWidget)
	{
		ItemDetailWidget->ClearItem();
	}

	if (Overlay_ItemDetail)
	{
		Overlay_ItemDetail->SetVisibility(ESlateVisibility::Collapsed);
	}
}
