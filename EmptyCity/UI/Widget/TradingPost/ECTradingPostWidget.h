// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECTradingPostWidget.generated.h"

UENUM(BlueprintType)
enum class ETradingMode : uint8
{
	Buy,
	Sell
};


struct FInventoryEntry;
class AECPlayerState;
class UWidgetSwitcher;
class UECInventoryItemInstance;
class UInventoryViewModel;
class UScrollBox;
class UWrapBox;
class UButton;
class UECTradingItemDetailWidget;
class UOverlay;
class UECInventorySlotWidget;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECTradingPostWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeDestruct() override;
	virtual void InjectViewModel(UMVVMViewModelBase* TargetViewModel) override;
	virtual void OnWidgetOpened_Implementation() override;
	virtual void OnWidgetClosed_Implementation() override;
	
	/** 인벤토리가 변경될 때마다 위젯을 업데이트 합니다. */
	void RefreshInventoryUI();

private:


	/** 지정한 ScrollBox에 인벤토리 엔트리를 표시합니다. */
	void RefreshInventoryScrollBox(UScrollBox* TargetScrollBox, const TArray<FInventoryEntry>& Entries);
	void RefreshPlayerSurvivalKitWrapBox(const TArray<FInventoryEntry>& Entries);
	UScrollBox* GetActiveInventoryScrollBox() const;
	UECTradingItemDetailWidget* GetActiveItemDetailWidget() const;
	void RefreshSelectionVisuals();
	void RefreshSelectedItemDetails();
	void ClearItemDetailWidgets();
	void RefreshConfirmTradeButtonState();
	void UnbindInventoryViewModels();

	void HandleItemSlotRightClicked(UECInventoryItemInstance* ItemInstance);
	void HandlePlayerSurvivalKitSlotClicked(UECInventoryItemInstance* ItemInstance);
	void HandlePlayerSurvivalKitSlotRightClicked(UECInventoryItemInstance* ItemInstance);
	int32 GetSelectedItemStackCount() const;
	int32 GetSelectedTradeQuantity() const;
	int32 GetMaximumTradeQuantity() const;
	int32 GetItemUnitPrice(const UECInventoryItemInstance* ItemInstance) const;
	int32 GetItemUnitPrice(const UECInventoryItemInstance* ItemInstance, ETradingMode TradingMode) const;
	bool IsEndingSubmissionItem(const UECInventoryItemInstance* ItemInstance) const;
	bool IsItemTradeAllowed(const UECInventoryItemInstance* ItemInstance, ETradingMode TradingMode) const;
	TArray<FInventoryEntry> GetTradeableEntries(
		const UInventoryViewModel* InventoryViewModel,
		ETradingMode TradingMode) const;
	bool CanTradeItem(UECInventoryItemInstance* ItemInstance, int32 Count = 1) const;
	bool ExecuteTrade(UECInventoryItemInstance* ItemInstance, int32 Count = 1);
	bool ExecuteEndingSubmission(UECInventoryItemInstance* ItemInstance);
	void RefreshAfterTrade(UECInventoryItemInstance* TradedItem);
	void HandleSelectedQuantityChanged(int32 NewQuantity);
	void HandlePlayerCoinChanged(int32 NewCoin);

	UFUNCTION()
	void HandleBuyTabClicked();
	
	UFUNCTION()
	void HandleSellTabClicked();
	
	UFUNCTION()
	void HandleConfirmTradeClicked();

	void HandleItemSlotClicked(UECInventoryItemInstance* ItemInstance);

	void ClearSelectedItem();

	void SetTradingMode(ETradingMode NewMode);

	UPROPERTY()
	TObjectPtr<UECInventoryItemInstance> SelectedItemInstance;

	UPROPERTY()
	TObjectPtr<UInventoryViewModel> CachedPlayerInventoryViewModel;

	UPROPERTY()
	TObjectPtr<UInventoryViewModel> CachedTradingPostInventoryViewModel;

	UPROPERTY()
	TWeakObjectPtr<AECPlayerState> CachedPlayerState;

	/** 현재 화면에 표시 중인 거래 모드입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	ETradingMode CurrentTradingMode = ETradingMode::Buy;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** ContentSwitcher 0번 구매 페이지의 거래소 인벤토리입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox_BuyInventory;

	/** ContentSwitcher 1번 판매 페이지의 플레이어 인벤토리입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox_SellInventory;

	/** 플레이어가 보유한 Survival Kit 아이템만 별도로 표시합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWrapBox> InventoryWrapBox_Player_SurvivalKit;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_BuyTab;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_SellTab;

	/** 0: 구매, 1: 판매 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> ContentSwitcher;

	/** 0: 구매용 디테일 위젯, 1: 판매용 디테일 위젯 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher_ItemDetail;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_ItemDetail;
	
	/** 구매 탭에서 사용할 아이템 상세 위젯입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UECTradingItemDetailWidget> ItemDetailWidget_Buy;

	/** 판매 탭에서 사용할 아이템 상세 위젯입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UECTradingItemDetailWidget> ItemDetailWidget_Sell;

	/** 인벤토리 슬롯 위젯에 사용할 블루프린트 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UECInventorySlotWidget> SlotWidgetClass;

	/** 플레이어 Survival Kit WrapBox에 사용할 플레이어 인벤토리형 슬롯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UECInventorySlotWidget> PlayerSurvivalKitSlotWidgetClass;

	/** 이 DT_Item 행은 일반 판매 대신 한 개를 제출하고 Cutscene.Ending을 실행합니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Ending")
	FName EndingSubmissionItemId = TEXT("Cylinder");

};
