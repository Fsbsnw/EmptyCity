// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECInventorySlotWidget.generated.h"

class UECInventoryItemInstance;
class UImage;
class UTextBlock;
class UECItemTooltipWidget;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECInventorySlotWidget : public UECUserWidget
{
	GENERATED_BODY()
public:
	/** 인벤토리가 변경될 때마다 슬롯 위젯을 업데이트 합니다. */
	void RefreshSlotUI(UECInventoryItemInstance* ItemInstance, int32 StackCount);
	
	/** 인벤토리 슬롯 위젯을 비어있는 상태로 변경합니다. */
	void SetEmptySlot();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StackCountText;

	/** 툴팁 위젯에 사용할 블루프린트 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Tooltip")
	TSubclassOf<UECItemTooltipWidget> TooltipWidgetClass;
};
