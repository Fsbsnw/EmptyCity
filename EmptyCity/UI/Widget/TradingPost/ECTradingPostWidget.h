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
class UTextBlock;
class UECInventoryItemInstance;
class UInventoryViewModel;
class UScrollBox;
class UButton;
class UECItemDetailWidget;
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


	/** 공통 ScrollBox에 현재 거래 모드의 인벤토리 엔트리를 표시합니다. */
	void RefreshInventoryScrollBox(UScrollBox* TargetScrollBox, const TArray<FInventoryEntry>& Entries);
	void RefreshSelectionVisuals();
	void UnbindInventoryViewModels();

	void HandleItemSlotRightClicked(UECInventoryItemInstance* ItemInstance);
	int32 GetSelectedItemStackCount() const;

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

	/** 현재 화면에 표시 중인 거래 모드입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	ETradingMode CurrentTradingMode = ETradingMode::Buy;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** 현재 탭에 맞는 거래소 또는 플레이어 인벤토리를 표시합니다. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox_Inventory;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_TradeValueLabel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton>	Button_ConfirmTrade;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Button_ConfirmTradeText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_BuyTab;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_SellTab;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_ItemDetail;
	
	/** 선택한 아이템의 공통 상세 정보를 표시합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UECItemDetailWidget> ItemDetailWidget;
	
	/** 인벤토리 슬롯 위젯에 사용할 블루프린트 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UECInventorySlotWidget> SlotWidgetClass;
};
