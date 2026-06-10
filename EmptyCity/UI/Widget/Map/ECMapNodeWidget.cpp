// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Map/ECMapNodeWidget.h"

FReply UECMapNodeWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 좌클릭인지 확인
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// 델리게이트에 바인딩된 함수들(맵 위젯)에게 내 정보를 전달
		OnNodeClicked.Broadcast(this);
        
		// 입력이 처리되었음을 엔진에 알림 (클릭 관통 방지)
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UECMapNodeWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	OnNodeHovered.Broadcast(this);
}

void UECMapNodeWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	OnNodeUnhovered.Broadcast(this);
}
