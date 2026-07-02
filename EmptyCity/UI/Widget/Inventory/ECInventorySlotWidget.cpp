// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Inventory/ECInventorySlotWidget.h"

#include "ECItemTooltipWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Inventory/Fragment/InventoryFragment_ItemSlotIcon.h"
#include "Subsystem/ECProgressionSubsystem.h"

void UECInventorySlotWidget::SetEmptySlot()
{
	// 빈 슬롯 처리: 투명하게 하거나 기본 배경 텍스처로 변경
	ItemIconImage->SetVisibility(ESlateVisibility::Hidden);
	StackCountText->SetVisibility(ESlateVisibility::Hidden);
	SetToolTip(nullptr); // 빈 슬롯은 툴팁 없음
}

void UECInventorySlotWidget::RefreshSlotUI(UECInventoryItemInstance* ItemInstance, int32 StackCount)
{
	// 1. 유효성 검사 및 빈 슬롯 처리
	if (!ItemInstance)
	{
		SetEmptySlot();
		return;
	}

	// 2. UI 데이터(프래그먼트) 가져오기 (없으면 진행 불가)
	const UInventoryFragment_ItemSlotIcon* SlotIconFragment = ItemInstance->FindFragmentByClass<UInventoryFragment_ItemSlotIcon>();
	if (!SlotIconFragment)
	{
		return;
	}

	// 3. 아이템 해금 상태 판별
	bool bIsUnlocked = true;
	if (UECProgressionSubsystem* ProgressionSubsystem = GetGameInstance()->GetSubsystem<UECProgressionSubsystem>())
	{
		bIsUnlocked = ProgressionSubsystem->IsItemUnlocked(ItemInstance->ItemDef);
	}

	// 4. 공통 UI 세팅 (아이콘 및 툴팁)
	ItemIconImage->SetVisibility(ESlateVisibility::Visible);
	ItemIconImage->SetBrush(SlotIconFragment->ItemBrush);
       
	if (TooltipWidgetClass)
	{
		UECItemTooltipWidget* TooltipWidget = CreateWidget<UECItemTooltipWidget>(this, TooltipWidgetClass);
       
		if (bIsUnlocked)
		{
			TooltipWidget->UpdateTooltipText(SlotIconFragment->ItemName, SlotIconFragment->ItemDescription);
		}
		else
		{
			TooltipWidget->UpdateTooltipText(SlotIconFragment->ItemName, SlotIconFragment->ItemLockedDescription);
		}
       
		this->SetToolTip(TooltipWidget);            
	}
       
	// 5. 해금 여부에 따른 세부 상태 적용 (잠금 vs 해금)
	if (!bIsUnlocked)
	{
		// [잠긴 상태] 아이콘 반투명, 수량 숨김
		ItemIconImage->SetOpacity(0.5f);
		StackCountText->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		// [해금 상태] 아이콘 불투명 복구, 수량 표시
		ItemIconImage->SetOpacity(1.0f); // 💡 재활용 시 원래 투명도로 돌려놓기 위해 필수!

		// 1개 초과일 때만 보이도록 수정
		if (StackCount > 1)
		{
			StackCountText->SetVisibility(ESlateVisibility::Visible);
			StackCountText->SetText(FText::AsNumber(StackCount));
		}
		else
		{
			StackCountText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
