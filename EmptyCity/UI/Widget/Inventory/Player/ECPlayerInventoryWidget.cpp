// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Inventory/Player/ECPlayerInventoryWidget.h"

#include "UI/Widget/Inventory/ECInventorySlotWidget.h"
#include "UI/Widget/Common/Item/ECItemDetailWidget.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/WrapBox.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "UI/ViewModel/InventoryInteractionViewModel.h"
#include "UI/ViewModel/InventoryViewModel.h"

void UECPlayerInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_MoveAllItemsToContainer)
	{
		Button_MoveAllItemsToContainer->OnClicked.RemoveDynamic(this, &ThisClass::HandleMoveAllItemsToContainerClicked);
		Button_MoveAllItemsToContainer->OnClicked.AddDynamic(this, &ThisClass::HandleMoveAllItemsToContainerClicked);
	}

	if (Button_MoveSelectedItemToContainer)
	{
		Button_MoveSelectedItemToContainer->OnClicked.RemoveDynamic(this, &ThisClass::HandleMoveSelectedItemToContainerClicked);
		Button_MoveSelectedItemToContainer->OnClicked.AddDynamic(this, &ThisClass::HandleMoveSelectedItemToContainerClicked);
	}

	if (Button_DiscardSelectedItem)
	{
		Button_DiscardSelectedItem->OnClicked.RemoveDynamic(this, &ThisClass::HandleDiscardSelectedItemClicked);
		Button_DiscardSelectedItem->OnClicked.AddDynamic(this, &ThisClass::HandleDiscardSelectedItemClicked);
	}
}

void UECPlayerInventoryWidget::NativeDestruct()
{
	UnbindInventoryViewModels();
	
	Super::NativeDestruct();
}

void UECPlayerInventoryWidget::OnWidgetOpened_Implementation()
{
	if (ItemDetailWidget)
	{
		Overlay_ItemDetail->SetVisibility(ESlateVisibility::Collapsed);
		ItemDetailWidget->ClearItem();
	}
	
	RefreshInventoryUI();
}

void UECPlayerInventoryWidget::UnbindInventoryViewModels()
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

FReply UECPlayerInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		ClearSelectedItem();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UECPlayerInventoryWidget::InjectViewModel(UMVVMViewModelBase* TargetViewModel)
{
	Super::InjectViewModel(TargetViewModel);

	UInventoryInteractionViewModel* InventoryInteractionViewModel = Cast<UInventoryInteractionViewModel>(TargetViewModel);
	if (!InventoryInteractionViewModel)
	{
		return;
	}

	CachedPlayerInventoryViewModel = InventoryInteractionViewModel->GetPlayerInventoryViewModel();
	CachedStorageInventoryViewModel = InventoryInteractionViewModel->GetStorageInventoryViewModel();

	if (CachedPlayerInventoryViewModel)
	{
		CachedPlayerInventoryViewModel->OnVMInventoryUpdated.AddUObject(this, &ThisClass::RefreshInventoryUI);
	}

	if (CachedStorageInventoryViewModel)
	{
		CachedStorageInventoryViewModel->OnVMInventoryUpdated.AddUObject(this, &ThisClass::RefreshInventoryUI);
	}
}

void UECPlayerInventoryWidget::RefreshInventoryUI()
{
	const UInventoryInteractionViewModel* InventoryInteractionViewModel = GetViewModel<UInventoryInteractionViewModel>();
	const bool bCanTransferItems = InventoryInteractionViewModel && InventoryInteractionViewModel->GetCanTransferItems();
	const ESlateVisibility ContainerVisibility = bCanTransferItems
		? ESlateVisibility::Visible
		: ESlateVisibility::Collapsed;

	if (InventoryWrapBox_Container)
	{
		InventoryWrapBox_Container->SetVisibility(ContainerVisibility);
	}

	if (Button_MoveAllItemsToContainer)
	{
		Button_MoveAllItemsToContainer->SetVisibility(ContainerVisibility);
		Button_MoveAllItemsToContainer->SetIsEnabled(bCanTransferItems);
	}

	if (Button_MoveSelectedItemToContainer)
	{
		Button_MoveSelectedItemToContainer->SetVisibility(ContainerVisibility);
	}

	if (!SlotWidgetClass)
	{
		return;
	}

	static const TArray<FInventoryEntry> EmptyEntries;
	const TArray<FInventoryEntry>& PlayerEntries = CachedPlayerInventoryViewModel
		? CachedPlayerInventoryViewModel->GetInventoryEntries()
		: EmptyEntries;
	
	const TArray<FInventoryEntry>& StorageEntries = CachedStorageInventoryViewModel
		? CachedStorageInventoryViewModel->GetInventoryEntries()
		: EmptyEntries;

	// 플레이어 슬롯은 우클릭 이동 가능
	RefreshInventoryWrapBox(InventoryWrapBox_Player,	PlayerEntries,true);
	// 보관함 슬롯은 별도의 선택 처리만 연결합니다.
	RefreshInventoryWrapBox(InventoryWrapBox_Container, StorageEntries,false);

	if (IsValid(SelectedItemInstance))
	{
		const UInventoryViewModel* SelectedInventoryViewModel =
			SelectedInventorySource == ESelectedInventorySource::Player
				? CachedPlayerInventoryViewModel.Get()
				: CachedStorageInventoryViewModel.Get();

		if (!SelectedInventoryViewModel || !SelectedInventoryViewModel->ContainsItem(SelectedItemInstance))
		{
			ClearSelectedItem();
		}
		else if (ItemDetailWidget)
		{
			ItemDetailWidget->SetItem(SelectedItemInstance, GetSelectedItemStackCount());
			Overlay_ItemDetail->SetVisibility(ESlateVisibility::Visible);
		}
	}

	RefreshSelectionVisuals();
}

void UECPlayerInventoryWidget::RefreshInventoryWrapBox(UWrapBox* TargetWrapBox, const TArray<FInventoryEntry>& Entries, bool bCanMoveToContainer)
{
	if (!TargetWrapBox)
	{
		return;
	}

	constexpr int32 MaxFixedSlots = 16;

	if (TargetWrapBox->GetChildrenCount() == 0)
	{
		for (int32 Index = 0; Index < MaxFixedSlots; ++Index)
		{
			UECInventorySlotWidget* SlotWidget = CreateWidget<UECInventorySlotWidget>(this, SlotWidgetClass);

			if (bCanMoveToContainer)
			{
				SlotWidget->OnClicked.BindUObject(this, &ThisClass::HandleInventorySlotClicked);
				SlotWidget->OnRightClicked.BindUObject(this, &ThisClass::HandleInventorySlotRightClicked);
			}
			else
			{
				SlotWidget->OnClicked.BindUObject(this, &ThisClass::HandleContainerInventorySlotClicked);
				SlotWidget->OnRightClicked.BindUObject(this, &ThisClass::HandleStorageSlotRightClicked);
			}
			
			TargetWrapBox->AddChildToWrapBox(SlotWidget);
		}
	}

	for (int32 Index = 0; Index < MaxFixedSlots; ++Index)
	{
		UECInventorySlotWidget* SlotWidget = Cast<UECInventorySlotWidget>(TargetWrapBox->GetChildAt(Index));

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

		const ESelectedInventorySource SlotInventorySource = bCanMoveToContainer
			? ESelectedInventorySource::Player
			: ESelectedInventorySource::Storage;
		const bool bIsSelected =
			SelectedInventorySource == SlotInventorySource &&
			IsValid(SelectedItemInstance) &&
			Entries.IsValidIndex(Index) &&
			Entries[Index].Instance == SelectedItemInstance;
		SlotWidget->SetSelected(bIsSelected);
	}
}

void UECPlayerInventoryWidget::RefreshSelectionVisuals()
{
	auto ApplySelection = [this](
		UWrapBox* TargetWrapBox,
		const UInventoryViewModel* InventoryViewModel,
		ESelectedInventorySource InventorySource)
	{
		if (!TargetWrapBox)
		{
			return;
		}

		static const TArray<FInventoryEntry> EmptyInventoryEntries;
		const TArray<FInventoryEntry>& Entries = InventoryViewModel
			? InventoryViewModel->GetInventoryEntries()
			: EmptyInventoryEntries;

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
			SlotWidget->SetSelected(bIsSelected);
		}
	};

	ApplySelection(
		InventoryWrapBox_Player,
		CachedPlayerInventoryViewModel,
		ESelectedInventorySource::Player);
	ApplySelection(
		InventoryWrapBox_Container,
		CachedStorageInventoryViewModel,
		ESelectedInventorySource::Storage);
}

void UECPlayerInventoryWidget::HandleInventorySlotClicked(UECInventoryItemInstance* ItemInstance)
{
	if (!IsValid(ItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	SelectedItemInstance = ItemInstance;
	SelectedInventorySource = ESelectedInventorySource::Player;
	if (ItemDetailWidget)
	{
		ItemDetailWidget->SetItem(ItemInstance, GetSelectedItemStackCount());
		Overlay_ItemDetail->SetVisibility(ESlateVisibility::Visible);
	}

	RefreshSelectionVisuals();
}

void UECPlayerInventoryWidget::HandleContainerInventorySlotClicked(UECInventoryItemInstance* ItemInstance)
{
	if (!IsValid(ItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	SelectedItemInstance = ItemInstance;
	SelectedInventorySource = ESelectedInventorySource::Storage;
	if (ItemDetailWidget)
	{
		ItemDetailWidget->SetItem(ItemInstance, GetSelectedItemStackCount());
		Overlay_ItemDetail->SetVisibility(ESlateVisibility::Visible);
	}

	RefreshSelectionVisuals();
}

int32 UECPlayerInventoryWidget::GetSelectedItemStackCount() const
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

	const FInventoryEntry* SelectedEntry = SelectedInventoryViewModel->GetInventoryEntries().FindByPredicate(
		[this](const FInventoryEntry& Entry)
		{
			return Entry.Instance == SelectedItemInstance;
		});

	return SelectedEntry ? FMath::Max(0, SelectedEntry->StackCount) : 0;
}

void UECPlayerInventoryWidget::HandleInventorySlotRightClicked(UECInventoryItemInstance* ItemInstance)
{
	if (UInventoryInteractionViewModel* InventoryInteractionViewModel = GetViewModel<UInventoryInteractionViewModel>())
	{
		InventoryInteractionViewModel->MoveItemToStorage(ItemInstance);
	}

	RefreshSelectionVisuals();
}

void UECPlayerInventoryWidget::HandleStorageSlotRightClicked(UECInventoryItemInstance* ItemInstance)
{
	if (UInventoryInteractionViewModel* InventoryInteractionViewModel = GetViewModel<UInventoryInteractionViewModel>())
	{
		InventoryInteractionViewModel->MoveItemToPlayer(ItemInstance);
	}

	RefreshSelectionVisuals();
}

void UECPlayerInventoryWidget::HandleMoveAllItemsToContainerClicked()
{
	UInventoryInteractionViewModel* InventoryInteractionViewModel = GetViewModel<UInventoryInteractionViewModel>();

	if (!InventoryInteractionViewModel)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("StorageInventoryViewModel이 주입되지 않았습니다."));
		return;
	}

	InventoryInteractionViewModel->MoveAllItemsToStorage();
	RefreshSelectionVisuals();
}

void UECPlayerInventoryWidget::HandleMoveSelectedItemToContainerClicked()
{
	if (!IsValid(SelectedItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	if (SelectedInventorySource == ESelectedInventorySource::None)
	{
		return;
	}

	if (UInventoryInteractionViewModel* InventoryInteractionViewModel = GetViewModel<UInventoryInteractionViewModel>())
	{
		if (SelectedInventorySource == ESelectedInventorySource::Player)
		{
			InventoryInteractionViewModel->MoveItemToStorage(SelectedItemInstance);
			if (!CachedPlayerInventoryViewModel || !CachedPlayerInventoryViewModel->ContainsItem(SelectedItemInstance))
			{
				ClearSelectedItem();
			}
			else
			{
				if (ItemDetailWidget)
				{
					ItemDetailWidget->SetItem(SelectedItemInstance, GetSelectedItemStackCount());
				}
				RefreshSelectionVisuals();
			}
		}
		else
		{
			InventoryInteractionViewModel->MoveItemToPlayer(SelectedItemInstance);
			if (!CachedStorageInventoryViewModel || !CachedStorageInventoryViewModel->ContainsItem(SelectedItemInstance))
			{
				ClearSelectedItem();
			}
			else
			{
				if (ItemDetailWidget)
				{
					ItemDetailWidget->SetItem(SelectedItemInstance, GetSelectedItemStackCount());
				}
				RefreshSelectionVisuals();
			}
		}
	}
	else
	{
		RefreshSelectionVisuals();
	}
}

void UECPlayerInventoryWidget::HandleDiscardSelectedItemClicked()
{
	if (!IsValid(SelectedItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	if (UInventoryInteractionViewModel* InventoryInteractionViewModel = GetViewModel<UInventoryInteractionViewModel>())
	{
		UInventoryViewModel* SelectedInventoryViewModel = nullptr;
		int32 RemovedCount = 0;

		if (SelectedInventorySource == ESelectedInventorySource::Player)
		{
			SelectedInventoryViewModel = CachedPlayerInventoryViewModel;
			RemovedCount = InventoryInteractionViewModel->DiscardPlayerItem(SelectedItemInstance);
		}
		else if (SelectedInventorySource == ESelectedInventorySource::Storage)
		{
			SelectedInventoryViewModel = CachedStorageInventoryViewModel;
			RemovedCount = InventoryInteractionViewModel->DiscardStorageItem(SelectedItemInstance);
		}

		if (RemovedCount > 0 &&
			(!SelectedInventoryViewModel || !SelectedInventoryViewModel->ContainsItem(SelectedItemInstance)))
		{
			ClearSelectedItem();
		}
		else
		{
			if (RemovedCount > 0 && ItemDetailWidget)
			{
				ItemDetailWidget->SetItem(SelectedItemInstance, GetSelectedItemStackCount());
			}
			RefreshSelectionVisuals();
		}
	}
	else
	{
		RefreshSelectionVisuals();
	}
}

void UECPlayerInventoryWidget::ClearSelectedItem()
{
	SelectedItemInstance = nullptr;
	SelectedInventorySource = ESelectedInventorySource::None;
	RefreshSelectionVisuals();

	if (ItemDetailWidget)
	{
		ItemDetailWidget->ClearItem();
		Overlay_ItemDetail->SetVisibility(ESlateVisibility::Collapsed);
	}
}
