// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/Inventory/ECInventoryDragDropOperation.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECInventorySlotWidget.generated.h"

class UECInventoryItemInstance;
class UImage;
class UTextBlock;
class UTexture2D;
class UECItemTooltipWidget;

DECLARE_DELEGATE_OneParam(FOnInventorySlotRightClicked, UECInventoryItemInstance*);
DECLARE_DELEGATE_OneParam(FOnInventorySlotClicked, UECInventoryItemInstance*);

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECInventorySlotWidget : public UECUserWidget
{
	GENERATED_BODY()
public:
	/** 인벤토리가 변경될 때마다 슬롯 위젯을 업데이트 합니다. */
	void RefreshSlotUI(UECInventoryItemInstance* ItemInstance, int32 StackCount, int32 Price = 0);
	
	/** 인벤토리 슬롯 위젯을 비어있는 상태로 변경합니다. */
	void SetEmptySlot();

	/** StorageBox에서만 드래그 시작 영역을 지정합니다. None이면 기존 슬롯처럼 드래그하지 않습니다. */
	void SetInventoryDragSource(EECInventoryDragSource NewDragSource);

	/** 선택 상태와 현재 스택 개수에 맞춰 슬롯 배경을 함께 갱신합니다. */
	void SetSelectedState(bool bNewSelected);

	/** 슬롯의 선택 상태에 맞춰 BP에서 선택 테두리를 갱신합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Selection")
	void SetSelected(bool bSelected);

	FOnInventorySlotClicked OnClicked;
	FOnInventorySlotRightClicked OnRightClicked;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry,	const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;

private:
	void RefreshSlotBackground();

	UPROPERTY()
	TObjectPtr<UECInventoryItemInstance> CachedItemInstance;

	int32 CachedStackCount = 0;
	int32 CachedPrice = 0;
	EECInventoryDragSource DragSource = EECInventoryDragSource::None;
	bool bIsSelected = false;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_ItemIcon;

	/** 단일/스택 및 선택 상태에 따라 교체할 슬롯 배경입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Slot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ItemName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ItemDescription;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ItemPrice;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ItemWeight;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_StackCount;

	/** 수량이 1일 때도 스택 개수 텍스트를 표시할지 결정합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Display")
	bool bShowStackCountWhenOne = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Display|Slot Background")
	TObjectPtr<UTexture2D> SingleNormalSlotImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Display|Slot Background")
	TObjectPtr<UTexture2D> SingleSelectedSlotImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Display|Slot Background")
	TObjectPtr<UTexture2D> StackNormalSlotImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Display|Slot Background")
	TObjectPtr<UTexture2D> StackSelectedSlotImage;

	/** 툴팁 위젯에 사용할 블루프린트 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Tooltip")
	TSubclassOf<UECItemTooltipWidget> TooltipWidgetClass;
};
