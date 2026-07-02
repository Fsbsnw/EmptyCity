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

void UECMapNodeWidget::SyncNodeState(bool bIsUnlocked, bool bIsNewReveal)
{
	// 1. 클릭 가능 여부 및 자물쇠 상태 업데이트
	UpdateUnlockVisual(bIsUnlocked);
    
	// 이 위젯 자체의 입력 가능 여부를 차단/허용
	SetIsEnabled(bIsUnlocked); 

	// 2. 만약 방금 막 해금된 곳이라면 반짝이는 연출 실행
	if (bIsNewReveal)
	{
		PlayNewRevealAnim();
	}
	// 3. 더 이상 새로운 해금이 아니라면, 루프 돌던 애니메이션을 강제로 꺼줍니다.
	else
	{
		StopNewRevealAnim();
	}
}
