// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Map/ECMapNodeWidget.h"

#include "Components/Button.h"

void UECMapNodeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_MapNode)
	{
		// NativeConstruct가 다시 호출돼도 중복 바인딩되지 않도록 제거 후 등록
		Button_MapNode->OnClicked.RemoveDynamic(this, &ThisClass::HandleNodeClicked);

		Button_MapNode->OnClicked.AddDynamic(this, &ThisClass::HandleNodeClicked);
	}
}

void UECMapNodeWidget::SyncNodeState(bool bIsUnlocked, bool bIsNewReveal)
{
	// 1. 클릭 가능 여부 및 자물쇠 상태 업데이트
	UpdateUnlockVisual(bIsUnlocked);

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

void UECMapNodeWidget::HandleNodeClicked()
{
	OnNodeClicked.Broadcast(this);
}