// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/TradingPost/ECTradingPostWidget.h"

#include "UI/Widget/Inventory/ECInventorySlotWidget.h"
#include "UI/Widget/Common/Item/ECItemDetailWidget.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "UI/ViewModel/InventoryInteractionViewModel.h"
#include "UI/ViewModel/InventoryViewModel.h"

void UECTradingPostWidget::OnWidgetOpened_Implementation()
{
	Super::OnWidgetOpened_Implementation();

	if (Button_BuyTab)
	{
		Button_BuyTab->OnClicked.RemoveDynamic(this, &ThisClass::HandleBuyTabClicked);
		Button_BuyTab->OnClicked.AddDynamic(this, &ThisClass::HandleBuyTabClicked);
	}

	if (Button_SellTab)
	{
		Button_SellTab->OnClicked.RemoveDynamic(this, &ThisClass::HandleSellTabClicked);
		Button_SellTab->OnClicked.AddDynamic(this, &ThisClass::HandleSellTabClicked);
	}

	if (Button_ConfirmTrade)
	{
		Button_ConfirmTrade->OnClicked.RemoveDynamic(this, &ThisClass::HandleConfirmTradeClicked);
		Button_ConfirmTrade->OnClicked.AddDynamic(this, &ThisClass::HandleConfirmTradeClicked);
	}

	if (ItemDetailWidget)
	{
		Overlay_ItemDetail->SetVisibility(ESlateVisibility::Collapsed);
		ItemDetailWidget->ClearItem();
	}

	SetTradingMode(ETradingMode::Buy);
}

void UECTradingPostWidget::OnWidgetClosed_Implementation()
{
	Super::OnWidgetClosed_Implementation();
}

void UECTradingPostWidget::NativeDestruct()
{
	UnbindInventoryViewModels();

	Super::NativeDestruct();
}

FReply UECTradingPostWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		ClearSelectedItem();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UECTradingPostWidget::InjectViewModel(UMVVMViewModelBase* TargetViewModel)
{
	Super::InjectViewModel(TargetViewModel);

	UInventoryInteractionViewModel* InventoryInteractionViewModel = Cast<UInventoryInteractionViewModel>(TargetViewModel);
	if (!InventoryInteractionViewModel)
	{
		return;
	}

	UnbindInventoryViewModels();

	CachedPlayerInventoryViewModel = InventoryInteractionViewModel->GetPlayerInventoryViewModel();
	CachedTradingPostInventoryViewModel = InventoryInteractionViewModel->GetStorageInventoryViewModel();

	if (CachedPlayerInventoryViewModel)
	{
		CachedPlayerInventoryViewModel->OnVMInventoryUpdated.AddUObject(this, &ThisClass::RefreshInventoryUI);
	}

	if (CachedTradingPostInventoryViewModel)
	{
		CachedTradingPostInventoryViewModel->OnVMInventoryUpdated.AddUObject(this, &ThisClass::RefreshInventoryUI);
	}

	RefreshInventoryUI();
}

void UECTradingPostWidget::UnbindInventoryViewModels()
{
	if (CachedPlayerInventoryViewModel)
	{
		CachedPlayerInventoryViewModel->OnVMInventoryUpdated.RemoveAll(this);
	}

	if (CachedTradingPostInventoryViewModel)
	{
		CachedTradingPostInventoryViewModel->OnVMInventoryUpdated.RemoveAll(this);
	}

	CachedPlayerInventoryViewModel = nullptr;
	CachedTradingPostInventoryViewModel = nullptr;
}

void UECTradingPostWidget::RefreshInventoryUI()
{
	if (!SlotWidgetClass)
	{
		return;
	}

	static const TArray<FInventoryEntry> EmptyEntries;
	const bool bIsBuyMode = CurrentTradingMode == ETradingMode::Buy;
	
	const UInventoryViewModel* DisplayInventoryViewModel = bIsBuyMode
		? CachedTradingPostInventoryViewModel.Get()
		: CachedPlayerInventoryViewModel.Get();
	
	const TArray<FInventoryEntry>& DisplayEntries = DisplayInventoryViewModel
		? DisplayInventoryViewModel->GetInventoryEntries()
		: EmptyEntries;

	RefreshInventoryScrollBox(ScrollBox_Inventory, DisplayEntries);

	if (IsValid(SelectedItemInstance))
	{
		const UInventoryViewModel* SelectedInventoryViewModel =
			bIsBuyMode
				? CachedTradingPostInventoryViewModel.Get()
				: CachedPlayerInventoryViewModel.Get();

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

void UECTradingPostWidget::RefreshInventoryScrollBox(UScrollBox* TargetScrollBox, const TArray<FInventoryEntry>& Entries)
{
	if (!TargetScrollBox)
	{
		return;
	}

	while (TargetScrollBox->GetChildrenCount() < Entries.Num())
	{
		UECInventorySlotWidget* SlotWidget = CreateWidget<UECInventorySlotWidget>(this, SlotWidgetClass);

		if (!SlotWidget)
		{
			break;
		}

		SlotWidget->OnClicked.BindUObject(this, &ThisClass::HandleItemSlotClicked);
		SlotWidget->OnRightClicked.BindUObject(this, &ThisClass::HandleItemSlotRightClicked);

		TargetScrollBox->AddChild(SlotWidget);
	}

	while (TargetScrollBox->GetChildrenCount() > Entries.Num())
	{
		TargetScrollBox->RemoveChildAt(TargetScrollBox->GetChildrenCount() - 1);
	}

	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		UECInventorySlotWidget* SlotWidget = Cast<UECInventorySlotWidget>(TargetScrollBox->GetChildAt(Index));

		if (!SlotWidget)
		{
			continue;
		}

		SlotWidget->RefreshSlotUI(Entries[Index].Instance, Entries[Index].StackCount);
		SlotWidget->SetSelected(
			IsValid(SelectedItemInstance) && Entries[Index].Instance == SelectedItemInstance);
	}
}

void UECTradingPostWidget::RefreshSelectionVisuals()
{
	if (!ScrollBox_Inventory)
	{
		return;
	}

	static const TArray<FInventoryEntry> EmptyEntries;
	const UInventoryViewModel* DisplayInventoryViewModel = CurrentTradingMode == ETradingMode::Buy
		? CachedTradingPostInventoryViewModel.Get()
		: CachedPlayerInventoryViewModel.Get();
	const TArray<FInventoryEntry>& Entries = DisplayInventoryViewModel
		? DisplayInventoryViewModel->GetInventoryEntries()
		: EmptyEntries;

	for (int32 Index = 0; Index < ScrollBox_Inventory->GetChildrenCount(); ++Index)
	{
		UECInventorySlotWidget* SlotWidget =
			Cast<UECInventorySlotWidget>(ScrollBox_Inventory->GetChildAt(Index));
		if (!SlotWidget)
		{
			continue;
		}

		const bool bIsSelected =
			IsValid(SelectedItemInstance) &&
			Entries.IsValidIndex(Index) &&
			Entries[Index].Instance == SelectedItemInstance;
		SlotWidget->SetSelected(bIsSelected);
	}
}

void UECTradingPostWidget::HandleItemSlotRightClicked(UECInventoryItemInstance* ItemInstance)
{
	if (!IsValid(ItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	if (UInventoryInteractionViewModel* InventoryInteractionViewModel = GetViewModel<UInventoryInteractionViewModel>())
	{
		if (CurrentTradingMode == ETradingMode::Sell)
		{
			InventoryInteractionViewModel->MoveItemToStorage(ItemInstance);
			if (ItemInstance == SelectedItemInstance &&
				(!CachedPlayerInventoryViewModel || !CachedPlayerInventoryViewModel->ContainsItem(ItemInstance)))
			{
				ClearSelectedItem();
			}
			else
			{
				RefreshSelectionVisuals();
			}
		}
		else
		{
			InventoryInteractionViewModel->MoveItemToPlayer(ItemInstance);
			if (ItemInstance == SelectedItemInstance &&
				(!CachedTradingPostInventoryViewModel || !CachedTradingPostInventoryViewModel->ContainsItem(ItemInstance)))
			{
				ClearSelectedItem();
			}
			else
			{
				RefreshSelectionVisuals();
			}
		}
	}
	else
	{
		RefreshSelectionVisuals();
	}
}

int32 UECTradingPostWidget::GetSelectedItemStackCount() const
{
	const UInventoryViewModel* SelectedInventoryViewModel = nullptr;
	if (CurrentTradingMode == ETradingMode::Sell)
	{
		SelectedInventoryViewModel = CachedPlayerInventoryViewModel;
	}
	else
	{
		SelectedInventoryViewModel = CachedTradingPostInventoryViewModel;
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

void UECTradingPostWidget::HandleBuyTabClicked()
{
	SetTradingMode(ETradingMode::Buy);
}

void UECTradingPostWidget::HandleSellTabClicked()
{
	SetTradingMode(ETradingMode::Sell);
}

void UECTradingPostWidget::HandleConfirmTradeClicked()
{
	UE_LOG(LogTemp,	Warning, TEXT("Trade Mode: %s"), CurrentTradingMode == ETradingMode::Buy ? TEXT("Buy") : TEXT("Sell"));

	if (!IsValid(SelectedItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	if (UInventoryInteractionViewModel* InventoryInteractionViewModel = GetViewModel<UInventoryInteractionViewModel>())
	{
		if (CurrentTradingMode == ETradingMode::Sell)
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
			if (!CachedTradingPostInventoryViewModel || !CachedTradingPostInventoryViewModel->ContainsItem(SelectedItemInstance))
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

void UECTradingPostWidget::HandleItemSlotClicked(UECInventoryItemInstance* ItemInstance)
{
	if (!IsValid(ItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	SelectedItemInstance = ItemInstance;
	if (Button_ConfirmTrade)
	{
		Button_ConfirmTrade->SetIsEnabled(true);
	}

	if (ItemDetailWidget)
	{
		ItemDetailWidget->SetItem(ItemInstance, GetSelectedItemStackCount());
		Overlay_ItemDetail->SetVisibility(ESlateVisibility::Visible);
	}

	RefreshSelectionVisuals();
}

void UECTradingPostWidget::ClearSelectedItem()
{
	SelectedItemInstance = nullptr;
	RefreshSelectionVisuals();

	if (Button_ConfirmTrade)
	{
		Button_ConfirmTrade->SetIsEnabled(false);
	}

	if (ItemDetailWidget)
	{
		ItemDetailWidget->ClearItem();
		Overlay_ItemDetail->SetVisibility(ESlateVisibility::Collapsed); 
	}
}

void UECTradingPostWidget::SetTradingMode(ETradingMode NewMode)
{
	CurrentTradingMode = NewMode;
	ClearSelectedItem();

	if (CurrentTradingMode == ETradingMode::Buy)
	{
		if (Text_TradeValueLabel)
		{
			Text_TradeValueLabel->SetText(FText::FromString(TEXT("구매 비용")));
		}

		if (Button_ConfirmTradeText)
		{
			Button_ConfirmTradeText->SetText(FText::FromString(TEXT("구매")));
		}
	}
	else
	{
		if (Text_TradeValueLabel)
		{
			Text_TradeValueLabel->SetText(FText::FromString(TEXT("판매 수익")));
		}

		if (Button_ConfirmTradeText)
		{
			Button_ConfirmTradeText->SetText(FText::FromString(TEXT("판매")));
		}
	}

	if (Button_BuyTab)
	{
		Button_BuyTab->SetIsEnabled(CurrentTradingMode != ETradingMode::Buy);
	}

	if (Button_SellTab)
	{
		Button_SellTab->SetIsEnabled(CurrentTradingMode != ETradingMode::Sell);
	}

	RefreshInventoryUI();

	if (ScrollBox_Inventory)
	{
		ScrollBox_Inventory->ScrollToStart();
	}
}
