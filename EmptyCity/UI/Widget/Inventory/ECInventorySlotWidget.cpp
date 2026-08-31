// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Inventory/ECInventorySlotWidget.h"

#include "Data/Item/ECItemTableRow.h"
#include "UI/Widget/Common/Item/ECItemTooltipWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Subsystem/ECItemDataSubsystem.h"
#include "Subsystem/ECProgressionSubsystem.h"

void UECInventorySlotWidget::SetEmptySlot()
{
	CachedItemInstance = nullptr;
	SetSelected(false);

	// 빈 슬롯 처리: 투명하게 하거나 기본 배경 텍스처로 변경
	Image_ItemIcon->SetVisibility(ESlateVisibility::Hidden);

	auto ClearTextBlock = [](UTextBlock* TextBlock)
	{
		if (TextBlock)
		{
			TextBlock->SetText(FText::GetEmpty());
			TextBlock->SetVisibility(ESlateVisibility::Hidden);
		}
	};

	ClearTextBlock(Text_ItemName);
	ClearTextBlock(Text_ItemDescription);
	ClearTextBlock(Text_ItemPrice);
	ClearTextBlock(Text_ItemWeight);
	ClearTextBlock(Text_StackCount);

	SetToolTip(nullptr); // 빈 슬롯은 툴팁 없음
}

FReply UECInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (IsValid(CachedItemInstance) && OnRightClicked.IsBound())
		{
			OnRightClicked.Execute(CachedItemInstance);
			return FReply::Handled();
		}
	}
	
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// 빈 슬롯은 nullptr를 전달해 부모 인벤토리 위젯의 현재 선택을 해제한다.
		OnClicked.ExecuteIfBound(CachedItemInstance);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UECInventorySlotWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (IsValid(CachedItemInstance) && OnRightClicked.IsBound())
		{
			OnRightClicked.Execute(CachedItemInstance);
			return FReply::Handled();
		}
	}
	
	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

void UECInventorySlotWidget::RefreshSlotUI(UECInventoryItemInstance* ItemInstance, int32 StackCount, int32 Price)
{
	// 1. 유효성 검사 및 빈 슬롯 처리
	if (!ItemInstance)
	{
		SetEmptySlot();
		return;
	}

	CachedItemInstance = ItemInstance;

	// 2. DT_Item에서 공통 UI 데이터를 가져옵니다.
	const FECItemTableRow* ItemData = UECItemDataSubsystem::Get(this).FindItemData(ItemInstance);
	if (!ItemData)
	{
		SetEmptySlot();
		return;
	}

	// 3. 아이템 해금 상태 판별
	bool bIsUnlocked = true;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UECProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UECProgressionSubsystem>())
		{
			bIsUnlocked = ProgressionSubsystem->IsItemUnlocked(ItemInstance->ItemDef);
		}
	}

	// 4. 공통 UI 세팅 (아이콘 및 툴팁)
	Image_ItemIcon->SetVisibility(ESlateVisibility::Visible);
	const TSoftObjectPtr<UTexture2D>& ItemIcon = !bIsUnlocked ? ItemData->LockedIcon : ItemData->Icon;
	if (!ItemIcon.IsNull())
	{
		Image_ItemIcon->SetBrushFromSoftTexture(ItemIcon, false);
	}
	else
	{
		Image_ItemIcon->SetBrush(FSlateBrush());
	}

	const FText& Description = bIsUnlocked || ItemData->LockedDescription.IsEmpty()
		? ItemData->Description
		: ItemData->LockedDescription;

	if (Text_ItemName)
	{
		Text_ItemName->SetText(ItemData->DisplayName);
		Text_ItemName->SetVisibility(ESlateVisibility::Visible);
	}

	if (Text_ItemDescription)
	{
		Text_ItemDescription->SetText(Description);
		Text_ItemDescription->SetVisibility(ESlateVisibility::Visible);
	}

	if (Text_ItemPrice)
	{
		Text_ItemPrice->SetText(FText::AsNumber(FMath::Max(0, Price)));
		Text_ItemPrice->SetVisibility(ESlateVisibility::Visible);
	}

	if (Text_ItemWeight)
	{
		FNumberFormattingOptions WeightFormatting;
		WeightFormatting.MinimumFractionalDigits = 0;
		WeightFormatting.MaximumFractionalDigits = 2;
		Text_ItemWeight->SetText(FText::AsNumber(ItemData->Weight, &WeightFormatting));
		Text_ItemWeight->SetVisibility(ESlateVisibility::Visible);
	}
       
	if (TooltipWidgetClass)
	{
		UECItemTooltipWidget* TooltipWidget = CreateWidget<UECItemTooltipWidget>(this, TooltipWidgetClass);
       
		if (bIsUnlocked)
		{
			TooltipWidget->UpdateTooltipText(ItemData->DisplayName, ItemData->Description);
		}
		else
		{
			TooltipWidget->UpdateTooltipText(ItemData->DisplayName, Description);
		}
       
		this->SetToolTip(TooltipWidget);            
	}
       
	// 5. 해금 여부에 따른 세부 상태 적용 (잠금 vs 해금)
	if (!bIsUnlocked)
	{
		// [잠긴 상태] 아이콘 반투명, 수량 숨김
		Image_ItemIcon->SetOpacity(0.5f);
		if (Text_StackCount)
		{
			Text_StackCount->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else
	{
		// [해금 상태] 아이콘 불투명 복구, 수량 표시
		Image_ItemIcon->SetOpacity(1.0f); // 재활용 시 원래 투명도로 돌려놓기 위해 필수!

		const bool bShouldShowStackCount = StackCount > 1 || (bShowStackCountWhenOne && StackCount == 1);

		if (Text_StackCount && bShouldShowStackCount)
		{
			Text_StackCount->SetVisibility(ESlateVisibility::Visible);
			Text_StackCount->SetText(FText::AsNumber(StackCount));
		}
		else if (Text_StackCount)
		{
			Text_StackCount->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
