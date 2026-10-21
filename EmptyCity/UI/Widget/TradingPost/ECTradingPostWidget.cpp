// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/TradingPost/ECTradingPostWidget.h"

#include "Actor/InventoryContainer/ECInventoryContainer.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "Character/Player/State/ECPlayerState.h"
#include "ECGameplayTags.h"
#include "UI/Widget/Inventory/ECInventorySlotWidget.h"
#include "UI/Widget/TradingPost/ECTradingItemDetailWidget.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/ScrollBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/WrapBox.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "Subsystem/ECCutsceneSubsystem.h"
#include "Subsystem/ECItemDataSubsystem.h"
#include "UI/Subsystem/UIManagerSubsystem.h"
#include "UI/ViewModel/InventoryInteractionViewModel.h"
#include "UI/ViewModel/InventoryViewModel.h"

DEFINE_LOG_CATEGORY_STATIC(LogECTradingPost, Log, All);

void UECTradingPostWidget::OnWidgetOpened_Implementation()
{
	Super::OnWidgetOpened_Implementation();

	if (AECPlayerState* PreviousPlayerState = CachedPlayerState.Get())
	{
		PreviousPlayerState->OnCoinChanged.RemoveAll(this);
	}
	CachedPlayerState = GetOwningPlayer()
		? GetOwningPlayer()->GetPlayerState<AECPlayerState>()
		: nullptr;
	if (AECPlayerState* PlayerState = CachedPlayerState.Get())
	{
		PlayerState->OnCoinChanged.AddUObject(this, &ThisClass::HandlePlayerCoinChanged);
	}

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

	if (ItemDetailWidget_Buy)
	{
		ItemDetailWidget_Buy->OnConfirmRequested.RemoveAll(this);
		ItemDetailWidget_Buy->OnConfirmRequested.AddUObject(
			this,
			&ThisClass::HandleConfirmTradeClicked);
		ItemDetailWidget_Buy->OnQuantityChanged.RemoveAll(this);
		ItemDetailWidget_Buy->OnQuantityChanged.AddUObject(
			this,
			&ThisClass::HandleSelectedQuantityChanged);
	}

	if (ItemDetailWidget_Sell)
	{
		ItemDetailWidget_Sell->OnConfirmRequested.RemoveAll(this);
		ItemDetailWidget_Sell->OnConfirmRequested.AddUObject(
			this,
			&ThisClass::HandleConfirmTradeClicked);
		ItemDetailWidget_Sell->OnQuantityChanged.RemoveAll(this);
		ItemDetailWidget_Sell->OnQuantityChanged.AddUObject(
			this,
			&ThisClass::HandleSelectedQuantityChanged);
	}

	Overlay_ItemDetail->SetVisibility(ESlateVisibility::Collapsed);
	ClearItemDetailWidgets();

	SetTradingMode(ETradingMode::Buy);
}

void UECTradingPostWidget::OnWidgetClosed_Implementation()
{
	if (AECPlayerState* PlayerState = CachedPlayerState.Get())
	{
		PlayerState->OnCoinChanged.RemoveAll(this);
	}
	CachedPlayerState.Reset();

	Super::OnWidgetClosed_Implementation();
}

void UECTradingPostWidget::NativeDestruct()
{
	UnbindInventoryViewModels();
	if (AECPlayerState* PlayerState = CachedPlayerState.Get())
	{
		PlayerState->OnCoinChanged.RemoveAll(this);
	}
	CachedPlayerState.Reset();
	if (ItemDetailWidget_Buy)
	{
		ItemDetailWidget_Buy->OnConfirmRequested.RemoveAll(this);
		ItemDetailWidget_Buy->OnQuantityChanged.RemoveAll(this);
	}
	if (ItemDetailWidget_Sell)
	{
		ItemDetailWidget_Sell->OnConfirmRequested.RemoveAll(this);
		ItemDetailWidget_Sell->OnQuantityChanged.RemoveAll(this);
	}

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

	TArray<FInventoryEntry> PlayerSurvivalKitEntries;
	if (CachedPlayerInventoryViewModel)
	{
		PlayerSurvivalKitEntries = CachedPlayerInventoryViewModel->GetInventoryEntriesByCategory(
			EECInventoryItemCategory::SurvivalKit);
	}

	const UInventoryViewModel* DisplayInventoryViewModel = bIsBuyMode
		? CachedTradingPostInventoryViewModel.Get()
		: CachedPlayerInventoryViewModel.Get();
	const TArray<FInventoryEntry> DisplayEntries = DisplayInventoryViewModel
		? GetTradeableEntries(DisplayInventoryViewModel, CurrentTradingMode)
		: EmptyEntries;

	RefreshInventoryScrollBox(GetActiveInventoryScrollBox(), DisplayEntries);
	RefreshPlayerSurvivalKitWrapBox(PlayerSurvivalKitEntries);

	if (IsValid(SelectedItemInstance))
	{
		const UInventoryViewModel* SelectedInventoryViewModel =
			bIsBuyMode
				? CachedTradingPostInventoryViewModel.Get()
				: CachedPlayerInventoryViewModel.Get();

		if (!SelectedInventoryViewModel
			|| !SelectedInventoryViewModel->ContainsItem(SelectedItemInstance)
			|| !IsItemTradeAllowed(SelectedItemInstance, CurrentTradingMode))
		{
			ClearSelectedItem();
		}
		else
		{
			if (UECTradingItemDetailWidget* ActiveItemDetailWidget = GetActiveItemDetailWidget())
			{
				ActiveItemDetailWidget->SetMaximumQuantity(GetMaximumTradeQuantity());
				RefreshSelectedItemDetails();
			}
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

		SlotWidget->RefreshSlotUI(
			Entries[Index].Instance,
			Entries[Index].StackCount,
			GetItemUnitPrice(Entries[Index].Instance));
		SlotWidget->SetSelectedState(
			IsValid(SelectedItemInstance) && Entries[Index].Instance == SelectedItemInstance);
	}
}

void UECTradingPostWidget::RefreshPlayerSurvivalKitWrapBox(const TArray<FInventoryEntry>& Entries)
{
	if (!InventoryWrapBox_Player_SurvivalKit)
	{
		return;
	}

	const TSubclassOf<UECInventorySlotWidget> SurvivalKitSlotClass =
		PlayerSurvivalKitSlotWidgetClass
			? PlayerSurvivalKitSlotWidgetClass
			: SlotWidgetClass;
	if (!SurvivalKitSlotClass)
	{
		return;
	}

	while (InventoryWrapBox_Player_SurvivalKit->GetChildrenCount() < Entries.Num())
	{
		UECInventorySlotWidget* SlotWidget = CreateWidget<UECInventorySlotWidget>(this, SurvivalKitSlotClass);
		if (!SlotWidget)
		{
			break;
		}

		SlotWidget->OnClicked.BindUObject(
			this,
			&ThisClass::HandlePlayerSurvivalKitSlotClicked);
		SlotWidget->OnRightClicked.BindUObject(
			this,
			&ThisClass::HandlePlayerSurvivalKitSlotRightClicked);
		InventoryWrapBox_Player_SurvivalKit->AddChild(SlotWidget);
	}

	while (InventoryWrapBox_Player_SurvivalKit->GetChildrenCount() > Entries.Num())
	{
		InventoryWrapBox_Player_SurvivalKit->RemoveChildAt(
			InventoryWrapBox_Player_SurvivalKit->GetChildrenCount() - 1);
	}

	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		UECInventorySlotWidget* SlotWidget = Cast<UECInventorySlotWidget>(
			InventoryWrapBox_Player_SurvivalKit->GetChildAt(Index));
		if (!SlotWidget)
		{
			continue;
		}

		SlotWidget->RefreshSlotUI(
			Entries[Index].Instance,
			Entries[Index].StackCount,
			GetItemUnitPrice(Entries[Index].Instance, ETradingMode::Sell));
		SlotWidget->SetSelectedState(
			CurrentTradingMode == ETradingMode::Sell &&
			IsValid(SelectedItemInstance) &&
			Entries[Index].Instance == SelectedItemInstance);
	}
}

UScrollBox* UECTradingPostWidget::GetActiveInventoryScrollBox() const
{
	return CurrentTradingMode == ETradingMode::Buy
		? ScrollBox_BuyInventory.Get()
		: ScrollBox_SellInventory.Get();
}

void UECTradingPostWidget::RefreshSelectionVisuals()
{
	UScrollBox* ActiveScrollBox = GetActiveInventoryScrollBox();
	static const TArray<FInventoryEntry> EmptyEntries;
	const UInventoryViewModel* DisplayInventoryViewModel = CurrentTradingMode == ETradingMode::Buy
		? CachedTradingPostInventoryViewModel.Get()
		: CachedPlayerInventoryViewModel.Get();
	const TArray<FInventoryEntry> Entries = DisplayInventoryViewModel
		? GetTradeableEntries(DisplayInventoryViewModel, CurrentTradingMode)
		: EmptyEntries;

	if (ActiveScrollBox)
	{
		for (int32 Index = 0; Index < ActiveScrollBox->GetChildrenCount(); ++Index)
		{
			UECInventorySlotWidget* SlotWidget =
				Cast<UECInventorySlotWidget>(ActiveScrollBox->GetChildAt(Index));
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
	}

	if (InventoryWrapBox_Player_SurvivalKit)
	{
		const TArray<FInventoryEntry> SurvivalKitEntries = CachedPlayerInventoryViewModel
			? CachedPlayerInventoryViewModel->GetInventoryEntriesByCategory(EECInventoryItemCategory::SurvivalKit)
			: EmptyEntries;

		for (int32 Index = 0;
			Index < InventoryWrapBox_Player_SurvivalKit->GetChildrenCount();
			++Index)
		{
			UECInventorySlotWidget* SlotWidget = Cast<UECInventorySlotWidget>(
				InventoryWrapBox_Player_SurvivalKit->GetChildAt(Index));
			if (!SlotWidget)
			{
				continue;
			}

			const bool bIsSelected =
				CurrentTradingMode == ETradingMode::Sell &&
				IsValid(SelectedItemInstance) &&
				SurvivalKitEntries.IsValidIndex(Index) &&
				SurvivalKitEntries[Index].Instance == SelectedItemInstance;
			SlotWidget->SetSelectedState(bIsSelected);
		}
	}
}

UECTradingItemDetailWidget* UECTradingPostWidget::GetActiveItemDetailWidget() const
{
	return CurrentTradingMode == ETradingMode::Buy
		? ItemDetailWidget_Buy.Get()
		: ItemDetailWidget_Sell.Get();
}

void UECTradingPostWidget::RefreshSelectedItemDetails()
{
	UECTradingItemDetailWidget* ActiveItemDetailWidget = GetActiveItemDetailWidget();
	if (!ActiveItemDetailWidget || !Overlay_ItemDetail || !IsValid(SelectedItemInstance))
	{
		return;
	}

	if (WidgetSwitcher_ItemDetail)
	{
		// 블루프린트의 자식 배치 순서에 의존하지 않고 현재 모드의 실제 위젯을 활성화합니다.
		WidgetSwitcher_ItemDetail->SetActiveWidget(ActiveItemDetailWidget);
		WidgetSwitcher_ItemDetail->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	const int64 TotalPrice =
		static_cast<int64>(GetItemUnitPrice(SelectedItemInstance)) * GetSelectedTradeQuantity();
	ActiveItemDetailWidget->SetItem(
		SelectedItemInstance,
		GetSelectedItemStackCount(),
		static_cast<int32>(FMath::Min<int64>(TotalPrice, MAX_int32)));

	const AECPlayerController* PlayerController = Cast<AECPlayerController>(GetOwningPlayer());
	const UECInventoryManagerComponent* PlayerInventory = PlayerController
		? PlayerController->GetInventoryManagerComponent()
		: nullptr;
	const AECPlayerState* PlayerState = CachedPlayerState.Get();
	const int64 CoinBeforeTrade = PlayerState ? PlayerState->GetCoin() : 0;
	const int64 CoinAfterTrade = CurrentTradingMode == ETradingMode::Buy
		? CoinBeforeTrade - TotalPrice
		: FMath::Min<int64>(CoinBeforeTrade + TotalPrice, MAX_int32);
	const float BagWeightBeforeTrade = PlayerInventory ? PlayerInventory->GetTotalWeight() : 0.0f;
	const double SelectedItemWeight = PlayerInventory
		? static_cast<double>(PlayerInventory->GetItemUnitWeight(SelectedItemInstance)) * GetSelectedTradeQuantity()
		: 0.0;
	const float BagWeightAfterTrade = static_cast<float>(
		CurrentTradingMode == ETradingMode::Buy
			? static_cast<double>(BagWeightBeforeTrade) + SelectedItemWeight
			: FMath::Max(0.0, static_cast<double>(BagWeightBeforeTrade) - SelectedItemWeight));
	ActiveItemDetailWidget->SetTradePreviewValues(
		TotalPrice,
		CoinBeforeTrade,
		CoinAfterTrade,
		BagWeightBeforeTrade,
		BagWeightAfterTrade);
	ActiveItemDetailWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	Overlay_ItemDetail->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	RefreshConfirmTradeButtonState();
}

void UECTradingPostWidget::ClearItemDetailWidgets()
{
	if (ItemDetailWidget_Buy)
	{
		ItemDetailWidget_Buy->ClearTradeItem();
	}

	if (ItemDetailWidget_Sell)
	{
		ItemDetailWidget_Sell->ClearTradeItem();
	}

}

void UECTradingPostWidget::RefreshConfirmTradeButtonState()
{
	if (UECTradingItemDetailWidget* ActiveItemDetailWidget = GetActiveItemDetailWidget())
	{
		const bool bCanTrade = CanTradeItem(SelectedItemInstance, GetSelectedTradeQuantity());
		ActiveItemDetailWidget->SetConfirmAvailable(bCanTrade);
	}
}

void UECTradingPostWidget::HandleItemSlotRightClicked(UECInventoryItemInstance* ItemInstance)
{
	if (!IsValid(ItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	// 엔딩 제출은 우클릭 즉시 판매하지 않고 반드시 상세 화면의 확인 버튼을 거칩니다.
	if (CurrentTradingMode == ETradingMode::Sell && IsEndingSubmissionItem(ItemInstance))
	{
		HandleItemSlotClicked(ItemInstance);
		return;
	}

	const int32 TradeQuantity = ItemInstance == SelectedItemInstance
		? GetSelectedTradeQuantity()
		: 1;
	ExecuteTrade(ItemInstance, TradeQuantity);
	RefreshAfterTrade(ItemInstance);
}

void UECTradingPostWidget::HandlePlayerSurvivalKitSlotClicked(UECInventoryItemInstance* ItemInstance)
{
	if (CurrentTradingMode == ETradingMode::Sell)
	{
		HandleItemSlotClicked(ItemInstance);
	}
}

void UECTradingPostWidget::HandlePlayerSurvivalKitSlotRightClicked(UECInventoryItemInstance* ItemInstance)
{
	if (CurrentTradingMode == ETradingMode::Sell)
	{
		HandleItemSlotRightClicked(ItemInstance);
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

int32 UECTradingPostWidget::GetSelectedTradeQuantity() const
{
	const UECTradingItemDetailWidget* ActiveItemDetailWidget = GetActiveItemDetailWidget();
	return ActiveItemDetailWidget
		? ActiveItemDetailWidget->GetSelectedQuantity()
		: FMath::Min(1, GetSelectedItemStackCount());
}

int32 UECTradingPostWidget::GetMaximumTradeQuantity() const
{
	const int32 StackCount = GetSelectedItemStackCount();
	return IsEndingSubmissionItem(SelectedItemInstance)
		? FMath::Min(StackCount, 1)
		: StackCount;
}

int32 UECTradingPostWidget::GetItemUnitPrice(const UECInventoryItemInstance* ItemInstance) const
{
	return GetItemUnitPrice(ItemInstance, CurrentTradingMode);
}

int32 UECTradingPostWidget::GetItemUnitPrice(
	const UECInventoryItemInstance* ItemInstance,
	ETradingMode TradingMode) const
{
	if (!IsValid(ItemInstance))
	{
		return 0;
	}

	const FECItemTableRow* ItemData = UECItemDataSubsystem::Get(this).FindItemData(ItemInstance);
	if (!ItemData || !IsItemTradeAllowed(ItemInstance, TradingMode))
	{
		return 0;
	}

	return FMath::Max(
		0,
		TradingMode == ETradingMode::Buy ? ItemData->BuyPrice : ItemData->SellPrice);
}

bool UECTradingPostWidget::IsItemTradeAllowed(
	const UECInventoryItemInstance* ItemInstance,
	ETradingMode TradingMode) const
{
	if (!IsValid(ItemInstance))
	{
		return false;
	}

	const FECItemTableRow* ItemData = UECItemDataSubsystem::Get(this).FindItemData(ItemInstance);
	if (!ItemData || ItemData->bPermanentPlayerItem)
	{
		return false;
	}

	if (TradingMode == ETradingMode::Sell && IsEndingSubmissionItem(ItemInstance))
	{
		return true;
	}

	return TradingMode == ETradingMode::Buy
		? ItemData->bCanBuy && ItemData->BuyPrice > 0
		: ItemData->bCanSell && ItemData->SellPrice > 0;
}

bool UECTradingPostWidget::IsEndingSubmissionItem(
	const UECInventoryItemInstance* ItemInstance) const
{
	return IsValid(ItemInstance)
		&& !EndingSubmissionItemId.IsNone()
		&& UECItemDataSubsystem::Get(this).FindItemId(ItemInstance)
			== EndingSubmissionItemId;
}

TArray<FInventoryEntry> UECTradingPostWidget::GetTradeableEntries(
	const UInventoryViewModel* InventoryViewModel,
	ETradingMode TradingMode) const
{
	TArray<FInventoryEntry> TradeableEntries;
	if (!InventoryViewModel)
	{
		return TradeableEntries;
	}

	for (const FInventoryEntry& Entry : InventoryViewModel->GetInventoryEntries())
	{
		if (Entry.StackCount > 0 && IsItemTradeAllowed(Entry.Instance, TradingMode))
		{
			TradeableEntries.Add(Entry);
		}
	}

	return TradeableEntries;
}

bool UECTradingPostWidget::CanTradeItem(UECInventoryItemInstance* ItemInstance, int32 Count) const
{
	if (!IsValid(ItemInstance)
		|| Count <= 0
		|| !GetViewModel<UInventoryInteractionViewModel>()
		|| !IsItemTradeAllowed(ItemInstance, CurrentTradingMode))
	{
		return false;
	}

	const UInventoryViewModel* SourceInventoryViewModel = CurrentTradingMode == ETradingMode::Sell
		? CachedPlayerInventoryViewModel.Get()
		: CachedTradingPostInventoryViewModel.Get();
	if (!SourceInventoryViewModel || !SourceInventoryViewModel->ContainsItem(ItemInstance))
	{
		return false;
	}

	const FInventoryEntry* SourceEntry = SourceInventoryViewModel->GetInventoryEntries().FindByPredicate(
		[ItemInstance](const FInventoryEntry& Entry)
		{
			return Entry.Instance == ItemInstance;
		});
	if (!SourceEntry || Count > SourceEntry->StackCount)
	{
		return false;
	}

	const AECPlayerController* PlayerController = Cast<AECPlayerController>(GetOwningPlayer());
	if (CurrentTradingMode == ETradingMode::Sell && IsEndingSubmissionItem(ItemInstance))
	{
		const UECInventoryManagerComponent* PlayerInventory = PlayerController
			? PlayerController->GetInventoryManagerComponent()
			: nullptr;
		return Count == 1
			&& PlayerInventory
			&& PlayerInventory->GetItemCountByDefinition(ItemInstance->ItemDef) >= 1;
	}

	const int32 UnitPrice = GetItemUnitPrice(ItemInstance);
	if (UnitPrice <= 0)
	{
		return false;
	}

	const int64 TotalPrice = static_cast<int64>(UnitPrice) * Count;
	if (TotalPrice > MAX_int32)
	{
		return false;
	}

	const AECPlayerState* PlayerState = PlayerController
		? PlayerController->GetPlayerState<AECPlayerState>()
		: nullptr;
	if (!PlayerState)
	{
		return false;
	}

	if (CurrentTradingMode == ETradingMode::Sell)
	{
		return true;
	}

	const UECInventoryManagerComponent* PlayerInventory = PlayerController
		? PlayerController->GetInventoryManagerComponent()
		: nullptr;
	return PlayerState->CanAfford(static_cast<int32>(TotalPrice))
		&& PlayerInventory
		&& PlayerInventory->CanAddItemDefinition(ItemInstance->ItemDef, Count);
}

bool UECTradingPostWidget::ExecuteTrade(UECInventoryItemInstance* ItemInstance, int32 Count)
{
	if (!CanTradeItem(ItemInstance, Count))
	{
		return false;
	}

	if (CurrentTradingMode == ETradingMode::Sell && IsEndingSubmissionItem(ItemInstance))
	{
		return ExecuteEndingSubmission(ItemInstance);
	}

	UInventoryInteractionViewModel* InventoryInteractionViewModel =
		GetViewModel<UInventoryInteractionViewModel>();
	APlayerController* PlayerController = GetOwningPlayer();
	AECPlayerState* PlayerState = PlayerController
		? PlayerController->GetPlayerState<AECPlayerState>()
		: nullptr;
	if (!InventoryInteractionViewModel || !PlayerState)
	{
		return false;
	}

	const int32 UnitPrice = GetItemUnitPrice(ItemInstance);
	const int32 TransferredCount = CurrentTradingMode == ETradingMode::Sell
		? InventoryInteractionViewModel->MoveItemToStorage(ItemInstance, Count)
		: InventoryInteractionViewModel->MoveItemToPlayer(ItemInstance, Count);
	if (TransferredCount <= 0)
	{
		return false;
	}

	const int32 TotalPrice = UnitPrice * TransferredCount;
	if (CurrentTradingMode == ETradingMode::Sell)
	{
		PlayerState->AddCoin(TotalPrice);
	}
	else
	{
		const bool bPaid = PlayerState->SpendCoin(TotalPrice);
		ensureMsgf(bPaid, TEXT("구매 가능 검사 후 아이템을 이동했지만 Coin 차감에 실패했습니다."));
	}

	return true;
}

bool UECTradingPostWidget::ExecuteEndingSubmission(UECInventoryItemInstance* ItemInstance)
{
	AECPlayerController* PlayerController = Cast<AECPlayerController>(GetOwningPlayer());
	UECInventoryManagerComponent* PlayerInventory = PlayerController
		? PlayerController->GetInventoryManagerComponent()
		: nullptr;
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	UECCutsceneSubsystem* CutsceneSubsystem = LocalPlayer
		? LocalPlayer->GetSubsystem<UECCutsceneSubsystem>()
		: nullptr;
	if (!IsValid(ItemInstance) || !PlayerInventory || !CutsceneSubsystem)
	{
		return false;
	}

	const TSubclassOf<UECInventoryItemDefinition> SubmittedItemDefinition = ItemInstance->ItemDef;
	if (!PlayerInventory->ConsumeItemsByDefinition(SubmittedItemDefinition, 1))
	{
		return false;
	}

	UInventoryInteractionViewModel* InventoryInteractionViewModel =
		GetViewModel<UInventoryInteractionViewModel>();
	AECInventoryContainer* TradingPost = InventoryInteractionViewModel
		? InventoryInteractionViewModel->GetStorageContainer()
		: nullptr;
	if (!CutsceneSubsystem->PlayCutsceneByTag(ECGameplayTags::Cutscene_Ending, TradingPost))
	{
		// 컷신 행/에셋이 아직 등록되지 않았거나 재생에 실패하면 제출을 취소합니다.
		const bool bRestored = PlayerInventory->AddItemDefinition(
			SubmittedItemDefinition,
			1,
			false) != nullptr;
		UE_LOG(
			LogECTradingPost,
			Error,
			TEXT("실린더 제출 후 Cutscene.Ending 재생에 실패했습니다. 아이템 복구: %s"),
			bRestored ? TEXT("성공") : TEXT("실패"));
		return false;
	}

	if (TradingPost)
	{
		if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(PlayerController))
		{
			UIManager->CloseDynamicWidget(TradingPost->InventoryUITag);
		}
	}

	return true;
}

void UECTradingPostWidget::RefreshAfterTrade(UECInventoryItemInstance* TradedItem)
{
	if (TradedItem != SelectedItemInstance)
	{
		RefreshSelectionVisuals();
		return;
	}

	const UInventoryViewModel* SourceInventoryViewModel = CurrentTradingMode == ETradingMode::Sell
		? CachedPlayerInventoryViewModel.Get()
		: CachedTradingPostInventoryViewModel.Get();
	if (!SourceInventoryViewModel || !SourceInventoryViewModel->ContainsItem(TradedItem))
	{
		ClearSelectedItem();
		return;
	}

	if (UECTradingItemDetailWidget* ActiveItemDetailWidget = GetActiveItemDetailWidget())
	{
		ActiveItemDetailWidget->SetMaximumQuantity(GetMaximumTradeQuantity());
	}
	RefreshSelectedItemDetails();
	RefreshSelectionVisuals();
}

void UECTradingPostWidget::HandleSelectedQuantityChanged(int32 NewQuantity)
{
	if (NewQuantity > 0 && IsValid(SelectedItemInstance))
	{
		RefreshSelectedItemDetails();
	}
}

void UECTradingPostWidget::HandlePlayerCoinChanged(int32 NewCoin)
{
	if (IsValid(SelectedItemInstance))
	{
		RefreshSelectedItemDetails();
	}
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
	if (!IsValid(SelectedItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	UECInventoryItemInstance* TradedItem = SelectedItemInstance;
	ExecuteTrade(TradedItem, GetSelectedTradeQuantity());
	RefreshAfterTrade(TradedItem);
}

void UECTradingPostWidget::HandleItemSlotClicked(UECInventoryItemInstance* ItemInstance)
{
	if (!IsValid(ItemInstance))
	{
		ClearSelectedItem();
		return;
	}

	SelectedItemInstance = ItemInstance;
	if (UECTradingItemDetailWidget* ActiveItemDetailWidget = GetActiveItemDetailWidget())
	{
		ActiveItemDetailWidget->ResetQuantityForSelection(GetMaximumTradeQuantity());
	}

	RefreshSelectedItemDetails();

	RefreshSelectionVisuals();
}

void UECTradingPostWidget::ClearSelectedItem()
{
	SelectedItemInstance = nullptr;
	RefreshSelectionVisuals();

	RefreshConfirmTradeButtonState();

	ClearItemDetailWidgets();
	Overlay_ItemDetail->SetVisibility(ESlateVisibility::Collapsed);
}

void UECTradingPostWidget::SetTradingMode(ETradingMode NewMode)
{
	CurrentTradingMode = NewMode;

	if (ContentSwitcher)
	{
		ContentSwitcher->SetActiveWidgetIndex(static_cast<int32>(CurrentTradingMode));
	}

	if (WidgetSwitcher_ItemDetail)
	{
		if (UECTradingItemDetailWidget* ActiveItemDetailWidget = GetActiveItemDetailWidget())
		{
			WidgetSwitcher_ItemDetail->SetActiveWidget(ActiveItemDetailWidget);
		}
	}

	ClearSelectedItem();

	if (Button_BuyTab)
	{
		Button_BuyTab->SetIsEnabled(CurrentTradingMode != ETradingMode::Buy);
	}

	if (Button_SellTab)
	{
		Button_SellTab->SetIsEnabled(CurrentTradingMode != ETradingMode::Sell);
	}

	RefreshInventoryUI();

	if (UScrollBox* ActiveScrollBox = GetActiveInventoryScrollBox())
	{
		ActiveScrollBox->ScrollToStart();
	}
}
