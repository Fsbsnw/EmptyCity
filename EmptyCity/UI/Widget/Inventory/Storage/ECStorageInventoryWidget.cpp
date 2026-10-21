// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/Inventory/Storage/ECStorageInventoryWidget.h"

#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/Widget.h"
#include "Components/WrapBox.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "Subsystem/ECSoundSubsystem.h"
#include "UI/ViewModel/InventoryInteractionViewModel.h"
#include "UI/ViewModel/InventoryViewModel.h"
#include "UI/Widget/Common/Item/ECItemDetailWidget.h"
#include "UI/Widget/Common/Item/ECItemQuantitySelectorWidget.h"
#include "UI/Widget/Inventory/ECInventoryDragDropOperation.h"
#include "UI/Widget/Inventory/ECInventorySlotWidget.h"

namespace StorageInventorySoundRows
{
	const FName MoveToContainerClick(TEXT("UI_InventorySlot_Button_MoveToContainer_Click"));

	void PlayMoveSound(const UObject* WorldContextObject)
	{
		UECSoundSubsystem::Get(WorldContextObject).PlaySFXByName(MoveToContainerClick);
	}
}

void UECStorageInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	if (Button_MoveAllItemsToContainer)
	{
		Button_MoveAllItemsToContainer->OnClicked.RemoveDynamic(
			this,
			&ThisClass::HandleMoveAllItemsToContainerClicked);
		Button_MoveAllItemsToContainer->OnClicked.AddDynamic(
			this,
			&ThisClass::HandleMoveAllItemsToContainerClicked);
	}

	if (Button_MoveSelectedItemToContainer)
	{
		Button_MoveSelectedItemToContainer->OnClicked.RemoveDynamic(
			this,
			&ThisClass::HandleMoveSelectedItemClicked);
		Button_MoveSelectedItemToContainer->OnClicked.AddDynamic(
			this,
			&ThisClass::HandleMoveSelectedItemClicked);
	}

}

void UECStorageInventoryWidget::NativeDestruct()
{
	UnbindInventoryViewModels();

	Super::NativeDestruct();
}

void UECStorageInventoryWidget::OnWidgetOpened_Implementation()
{
	Super::OnWidgetOpened_Implementation();

	SetKeyboardFocus();
	ClearSelectedItem();
	RefreshInventoryUI();
}

void UECStorageInventoryWidget::UnbindInventoryViewModels()
{
	if (CachedPlayerInventoryViewModel)
	{
		CachedPlayerInventoryViewModel->OnVMInventoryUpdated.RemoveAll(this);
	}

	if (CachedStorageInventoryViewModel)
	{
		CachedStorageInventoryViewModel->OnVMInventoryUpdated.RemoveAll(this);
	}

	CachedPlayerInventoryViewModel = nullptr;
	CachedStorageInventoryViewModel = nullptr;
}

FReply UECStorageInventoryWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		ClearSelectedItem();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UECStorageInventoryWidget::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::G)
	{
		HandleMoveSelectedItemClicked();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

bool UECStorageInventoryWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const UECInventoryDragDropOperation* InventoryDragOperation =
		Cast<UECInventoryDragDropOperation>(InOperation);
	if (!InventoryDragOperation || !IsValid(InventoryDragOperation->GetItemInstance()))
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	const ESelectedInventorySource DropTarget = ResolveDropTarget(
		InDragDropEvent.GetScreenSpacePosition());
	const ESelectedInventorySource DragSource =
		InventoryDragOperation->GetSource() == EECInventoryDragSource::Player
			? ESelectedInventorySource::Player
			: InventoryDragOperation->GetSource() == EECInventoryDragSource::Storage
				? ESelectedInventorySource::Storage
				: ESelectedInventorySource::None;

	if (DropTarget == ESelectedInventorySource::None)
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}
	if (DropTarget == DragSource)
	{
		// 같은 인벤토리 안에서의 드롭은 슬롯 재정렬 없이 소비합니다.
		return true;
	}

	UInventoryInteractionViewModel* InventoryInteractionViewModel =
		GetViewModel<UInventoryInteractionViewModel>();
	if (!InventoryInteractionViewModel)
	{
		return true;
	}

	UECInventoryItemInstance* ItemInstance = InventoryDragOperation->GetItemInstance();
	// 드래그 이동은 수량 선택기와 무관하게 해당 슬롯의 전체 스택을 옮깁니다.
	const int32 MoveCount = InventoryDragOperation->GetStackCount();

	int32 MovedCount = 0;
	if (DragSource == ESelectedInventorySource::Player
		&& DropTarget == ESelectedInventorySource::Storage)
	{
		MovedCount = InventoryInteractionViewModel->MoveItemToStorage(ItemInstance, MoveCount);
	}
	else if (DragSource == ESelectedInventorySource::Storage
		&& DropTarget == ESelectedInventorySource::Player)
	{
		MovedCount = InventoryInteractionViewModel->MoveItemToPlayer(ItemInstance, MoveCount);
	}

	if (MovedCount > 0)
	{
		StorageInventorySoundRows::PlayMoveSound(this);
	}

	return true;
}

void UECStorageInventoryWidget::InjectViewModel(UMVVMViewModelBase* TargetViewModel)
{
	Super::InjectViewModel(TargetViewModel);

	UInventoryInteractionViewModel* InventoryInteractionViewModel =
		Cast<UInventoryInteractionViewModel>(TargetViewModel);
	if (!InventoryInteractionViewModel)
	{
		return;
	}

	UnbindInventoryViewModels();
	CachedPlayerInventoryViewModel = InventoryInteractionViewModel->GetPlayerInventoryViewModel();
	CachedStorageInventoryViewModel = InventoryInteractionViewModel->GetStorageInventoryViewModel();

	if (CachedPlayerInventoryViewModel)
	{
		CachedPlayerInventoryViewModel->OnVMInventoryUpdated.AddUObject(
			this,
			&ThisClass::RefreshInventoryUI);
	}

	if (CachedStorageInventoryViewModel)
	{
		CachedStorageInventoryViewModel->OnVMInventoryUpdated.AddUObject(
			this,
			&ThisClass::RefreshInventoryUI);
	}
}

void UECStorageInventoryWidget::RefreshInventoryUI()
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
	const TArray<FInventoryEntry>& StorageEntries = CachedStorageInventoryViewModel
		? CachedStorageInventoryViewModel->GetInventoryEntries()
		: EmptyEntries;

	RefreshInventoryWrapBox(
		InventoryWrapBox_Player_ToolWeapon,
		ToolWeaponEntries,
		ESelectedInventorySource::Player);
	RefreshInventoryWrapBox(
		InventoryWrapBox_Player_SurvivalKit,
		SurvivalKitEntries,
		ESelectedInventorySource::Player);
	RefreshInventoryWrapBox(
		InventoryWrapBox_Player_Item,
		ItemEntries,
		ESelectedInventorySource::Player);
	RefreshInventoryWrapBox(
		InventoryWrapBox_Container,
		StorageEntries,
		ESelectedInventorySource::Storage);

	if (IsValid(SelectedItemInstance))
	{
		const UInventoryViewModel* SelectedInventoryViewModel =
			SelectedInventorySource == ESelectedInventorySource::Player
				? CachedPlayerInventoryViewModel.Get()
				: CachedStorageInventoryViewModel.Get();

		if (!SelectedInventoryViewModel ||
			!SelectedInventoryViewModel->ContainsItem(SelectedItemInstance))
		{
			ClearSelectedItem();
		}
		else
		{
			if (UECItemQuantitySelectorWidget* QuantitySelector = GetSelectedItemQuantitySelector())
			{
				QuantitySelector->SetMaximumQuantity(GetSelectedItemStackCount());
			}
			RefreshSelectedItemDetail();
		}
	}

	RefreshSelectionVisuals();
}

void UECStorageInventoryWidget::RefreshInventoryWrapBox(
	UWrapBox* TargetWrapBox,
	const TArray<FInventoryEntry>& Entries,
	ESelectedInventorySource InventorySource)
{
	if (!TargetWrapBox)
	{
		return;
	}

	constexpr int32 MaxFixedStorageSlots = 16;
	const int32 DesiredSlotCount = InventorySource == ESelectedInventorySource::Storage
		? MaxFixedStorageSlots
		: Entries.Num();

	while (TargetWrapBox->GetChildrenCount() < DesiredSlotCount)
	{
		UECInventorySlotWidget* SlotWidget =
			CreateWidget<UECInventorySlotWidget>(this, SlotWidgetClass);
		if (!SlotWidget)
		{
			break;
		}

		if (InventorySource == ESelectedInventorySource::Player)
		{
			SlotWidget->SetInventoryDragSource(EECInventoryDragSource::Player);
			SlotWidget->OnClicked.BindUObject(this, &ThisClass::HandlePlayerSlotClicked);
			SlotWidget->OnRightClicked.BindUObject(this, &ThisClass::HandlePlayerSlotRightClicked);
		}
		else
		{
			SlotWidget->SetInventoryDragSource(EECInventoryDragSource::Storage);
			SlotWidget->OnClicked.BindUObject(this, &ThisClass::HandleStorageSlotClicked);
			SlotWidget->OnRightClicked.BindUObject(this, &ThisClass::HandleStorageSlotRightClicked);
		}

		TargetWrapBox->AddChildToWrapBox(SlotWidget);
	}

	while (TargetWrapBox->GetChildrenCount() > DesiredSlotCount)
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
			SlotWidget->SetInventoryDragSource(
				InventorySource == ESelectedInventorySource::Player
					? EECInventoryDragSource::Player
					: EECInventoryDragSource::Storage);
			SlotWidget->RefreshSlotUI(Entries[Index].Instance, Entries[Index].StackCount);
		}
		else
		{
			SlotWidget->SetEmptySlot();
		}

		const bool bIsSelected =
			SelectedInventorySource == InventorySource &&
			IsValid(SelectedItemInstance) &&
			Entries.IsValidIndex(Index) &&
			Entries[Index].Instance == SelectedItemInstance;
		SlotWidget->SetSelectedState(bIsSelected);
	}
}

void UECStorageInventoryWidget::RefreshSelectionVisuals()
{
	auto ApplySelection = [this](
		UWrapBox* TargetWrapBox,
		const TArray<FInventoryEntry>& Entries,
		ESelectedInventorySource InventorySource)
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
				SelectedInventorySource == InventorySource &&
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
	const TArray<FInventoryEntry>& StorageEntries = CachedStorageInventoryViewModel
		? CachedStorageInventoryViewModel->GetInventoryEntries()
		: EmptyEntries;

	ApplySelection(
		InventoryWrapBox_Player_ToolWeapon,
		ToolWeaponEntries,
		ESelectedInventorySource::Player);
	ApplySelection(
		InventoryWrapBox_Player_SurvivalKit,
		SurvivalKitEntries,
		ESelectedInventorySource::Player);
	ApplySelection(
		InventoryWrapBox_Player_Item,
		ItemEntries,
		ESelectedInventorySource::Player);
	ApplySelection(
		InventoryWrapBox_Container,
		StorageEntries,
		ESelectedInventorySource::Storage);
}

void UECStorageInventoryWidget::HandlePlayerSlotClicked(UECInventoryItemInstance* ItemInstance)
{
	if (!IsValid(ItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	const bool bSelectionChanged =
		SelectedItemInstance != ItemInstance
		|| SelectedInventorySource != ESelectedInventorySource::Player;
	SelectedItemInstance = ItemInstance;
	SelectedInventorySource = ESelectedInventorySource::Player;
	if (!bSelectionChanged)
	{
		RefreshSelectionVisuals();
		return;
	}
	ClearItemQuantitySelectors();
	if (UECItemQuantitySelectorWidget* QuantitySelector = GetSelectedItemQuantitySelector())
	{
		QuantitySelector->ResetForSelection(GetSelectedItemStackCount());
	}
	RefreshSelectedItemDetail();

	RefreshSelectionVisuals();
}

void UECStorageInventoryWidget::HandleStorageSlotClicked(UECInventoryItemInstance* ItemInstance)
{
	if (!IsValid(ItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	const bool bSelectionChanged =
		SelectedItemInstance != ItemInstance
		|| SelectedInventorySource != ESelectedInventorySource::Storage;
	SelectedItemInstance = ItemInstance;
	SelectedInventorySource = ESelectedInventorySource::Storage;
	if (!bSelectionChanged)
	{
		RefreshSelectionVisuals();
		return;
	}
	ClearItemQuantitySelectors();
	if (UECItemQuantitySelectorWidget* QuantitySelector = GetSelectedItemQuantitySelector())
	{
		QuantitySelector->ResetForSelection(GetSelectedItemStackCount());
	}
	RefreshSelectedItemDetail();

	RefreshSelectionVisuals();
}

UECItemDetailWidget* UECStorageInventoryWidget::GetSelectedItemDetailWidget() const
{
	if (SelectedInventorySource == ESelectedInventorySource::Player && ItemDetailWidget_Right)
	{
		return ItemDetailWidget_Right;
	}

	if (SelectedInventorySource == ESelectedInventorySource::Storage && ItemDetailWidget_Left)
	{
		return ItemDetailWidget_Left;
	}

	return ItemDetailWidget;
}

UOverlay* UECStorageInventoryWidget::GetSelectedItemDetailOverlay() const
{
	if (SelectedInventorySource == ESelectedInventorySource::Player && Overlay_ItemDetail_Right)
	{
		return Overlay_ItemDetail_Right;
	}

	if (SelectedInventorySource == ESelectedInventorySource::Storage && Overlay_ItemDetail_Left)
	{
		return Overlay_ItemDetail_Left;
	}

	return Overlay_ItemDetail;
}

UECItemQuantitySelectorWidget* UECStorageInventoryWidget::GetSelectedItemQuantitySelector() const
{
	if (SelectedInventorySource == ESelectedInventorySource::Player && ItemQuantitySelector_Right)
	{
		return ItemQuantitySelector_Right;
	}

	if (SelectedInventorySource == ESelectedInventorySource::Storage && ItemQuantitySelector_Left)
	{
		return ItemQuantitySelector_Left;
	}

	return ItemQuantitySelector;
}

void UECStorageInventoryWidget::ClearItemQuantitySelectors()
{
	auto ClearQuantitySelector = [](UECItemQuantitySelectorWidget* QuantitySelector)
	{
		if (QuantitySelector)
		{
			QuantitySelector->ResetForSelection(0);
		}
	};

	ClearQuantitySelector(ItemQuantitySelector_Left);
	ClearQuantitySelector(ItemQuantitySelector_Right);
	ClearQuantitySelector(ItemQuantitySelector);
}

void UECStorageInventoryWidget::ClearItemDetailWidgets()
{
	auto ClearDetailWidget = [](UECItemDetailWidget* DetailWidget)
	{
		if (DetailWidget)
		{
			DetailWidget->ClearItem();
		}
	};

	auto CollapseDetailOverlay = [](UOverlay* DetailOverlay)
	{
		if (DetailOverlay)
		{
			DetailOverlay->SetVisibility(ESlateVisibility::Collapsed);
		}
	};

	ClearDetailWidget(ItemDetailWidget_Left);
	ClearDetailWidget(ItemDetailWidget_Right);
	ClearDetailWidget(ItemDetailWidget);
	CollapseDetailOverlay(Overlay_ItemDetail_Left);
	CollapseDetailOverlay(Overlay_ItemDetail_Right);
	CollapseDetailOverlay(Overlay_ItemDetail);
}

void UECStorageInventoryWidget::RefreshSelectedItemDetail()
{
	ClearItemDetailWidgets();

	if (!IsValid(SelectedItemInstance))
	{
		return;
	}

	if (UECItemDetailWidget* DetailWidget = GetSelectedItemDetailWidget())
	{
		DetailWidget->SetItem(SelectedItemInstance, GetSelectedItemStackCount());
	}

	if (UOverlay* DetailOverlay = GetSelectedItemDetailOverlay())
	{
		DetailOverlay->SetVisibility(ESlateVisibility::Visible);
	}
}

int32 UECStorageInventoryWidget::GetSelectedItemStackCount() const
{
	const UInventoryViewModel* SelectedInventoryViewModel = nullptr;
	if (SelectedInventorySource == ESelectedInventorySource::Player)
	{
		SelectedInventoryViewModel = CachedPlayerInventoryViewModel;
	}
	else if (SelectedInventorySource == ESelectedInventorySource::Storage)
	{
		SelectedInventoryViewModel = CachedStorageInventoryViewModel;
	}

	if (!SelectedInventoryViewModel || !IsValid(SelectedItemInstance))
	{
		return 0;
	}

	const FInventoryEntry* SelectedEntry =
		SelectedInventoryViewModel->GetInventoryEntries().FindByPredicate(
			[this](const FInventoryEntry& Entry)
			{
				return Entry.Instance == SelectedItemInstance;
			});

	return SelectedEntry ? FMath::Max(0, SelectedEntry->StackCount) : 0;
}

int32 UECStorageInventoryWidget::GetSelectedActionQuantity() const
{
	const UECItemQuantitySelectorWidget* QuantitySelector = GetSelectedItemQuantitySelector();
	return QuantitySelector
		? QuantitySelector->GetSelectedQuantity()
		: FMath::Min(1, GetSelectedItemStackCount());
}

UECStorageInventoryWidget::ESelectedInventorySource
UECStorageInventoryWidget::ResolveDropTarget(const FVector2D& ScreenPosition) const
{
	const UWidget* StorageDropTarget = DropTarget_StorageInventory
		? DropTarget_StorageInventory.Get()
		: InventoryWrapBox_Container.Get();
	if (StorageDropTarget
		&& StorageDropTarget->GetCachedGeometry().IsUnderLocation(ScreenPosition))
	{
		return ESelectedInventorySource::Storage;
	}

	const bool bOverPlayerInventory =
		(DropTarget_PlayerInventory
			&& DropTarget_PlayerInventory->GetCachedGeometry().IsUnderLocation(ScreenPosition))
		|| (!DropTarget_PlayerInventory && InventoryWrapBox_Player_ToolWeapon
			&& InventoryWrapBox_Player_ToolWeapon->GetCachedGeometry().IsUnderLocation(ScreenPosition))
		|| (!DropTarget_PlayerInventory && InventoryWrapBox_Player_SurvivalKit
			&& InventoryWrapBox_Player_SurvivalKit->GetCachedGeometry().IsUnderLocation(ScreenPosition))
		|| (!DropTarget_PlayerInventory && InventoryWrapBox_Player_Item
			&& InventoryWrapBox_Player_Item->GetCachedGeometry().IsUnderLocation(ScreenPosition));
	return bOverPlayerInventory
		? ESelectedInventorySource::Player
		: ESelectedInventorySource::None;
}

void UECStorageInventoryWidget::HandlePlayerSlotRightClicked(UECInventoryItemInstance* ItemInstance)
{
	if (UInventoryInteractionViewModel* InventoryInteractionViewModel =
		GetViewModel<UInventoryInteractionViewModel>())
	{
		const int32 MoveCount =
			SelectedInventorySource == ESelectedInventorySource::Player &&
			SelectedItemInstance == ItemInstance
				? GetSelectedActionQuantity()
				: 1;
		const int32 MovedCount = InventoryInteractionViewModel->MoveItemToStorage(ItemInstance, MoveCount);
		if (MovedCount > 0)
		{
			StorageInventorySoundRows::PlayMoveSound(this);
		}
	}
}

void UECStorageInventoryWidget::HandleStorageSlotRightClicked(UECInventoryItemInstance* ItemInstance)
{
	if (UInventoryInteractionViewModel* InventoryInteractionViewModel =
		GetViewModel<UInventoryInteractionViewModel>())
	{
		const int32 MoveCount =
			SelectedInventorySource == ESelectedInventorySource::Storage &&
			SelectedItemInstance == ItemInstance
				? GetSelectedActionQuantity()
				: 1;
		const int32 MovedCount = InventoryInteractionViewModel->MoveItemToPlayer(ItemInstance, MoveCount);
		if (MovedCount > 0)
		{
			StorageInventorySoundRows::PlayMoveSound(this);
		}
	}
}

void UECStorageInventoryWidget::HandleMoveAllItemsToContainerClicked()
{
	UInventoryInteractionViewModel* InventoryInteractionViewModel =
		GetViewModel<UInventoryInteractionViewModel>();
	if (!InventoryInteractionViewModel)
	{
		return;
	}

	const int32 MovedCount = InventoryInteractionViewModel->MoveAllItemsToStorage();
	if (MovedCount > 0)
	{
		StorageInventorySoundRows::PlayMoveSound(this);
	}
}

void UECStorageInventoryWidget::HandleMoveSelectedItemClicked()
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

	UInventoryViewModel* SourceInventoryViewModel = nullptr;
	int32 MovedCount = 0;

	if (SelectedInventorySource == ESelectedInventorySource::Player)
	{
		SourceInventoryViewModel = CachedPlayerInventoryViewModel;
		MovedCount = InventoryInteractionViewModel->MoveItemToStorage(
			SelectedItemInstance,
			GetSelectedActionQuantity());
	}
	else if (SelectedInventorySource == ESelectedInventorySource::Storage)
	{
		SourceInventoryViewModel = CachedStorageInventoryViewModel;
		MovedCount = InventoryInteractionViewModel->MoveItemToPlayer(
			SelectedItemInstance,
			GetSelectedActionQuantity());
	}

	if (MovedCount <= 0)
	{
		return;
	}

	StorageInventorySoundRows::PlayMoveSound(this);

	if (!SourceInventoryViewModel || !SourceInventoryViewModel->ContainsItem(SelectedItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	RefreshSelectedItemDetail();
	if (UECItemQuantitySelectorWidget* QuantitySelector = GetSelectedItemQuantitySelector())
	{
		QuantitySelector->SetMaximumQuantity(GetSelectedItemStackCount());
	}
	RefreshSelectionVisuals();
}

void UECStorageInventoryWidget::ClearSelectedItem()
{
	SelectedItemInstance = nullptr;
	SelectedInventorySource = ESelectedInventorySource::None;
	ClearItemQuantitySelectors();
	RefreshSelectionVisuals();
	ClearItemDetailWidgets();
}
