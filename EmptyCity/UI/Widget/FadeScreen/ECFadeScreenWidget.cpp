// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/FadeScreen/ECFadeScreenWidget.h"


void UECFadeScreenWidget::PlayTransition(float AnimSpeed)
{
	if (FadeAnim)
	{
		PlayAnimation(FadeAnim, 0.f, 1, EUMGSequencePlayMode::Forward, AnimSpeed);
	}
}

void UECFadeScreenWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);

	// 페이드 애니메이션 확인
	if (Animation == FadeAnim)
	{
		// 페이드 애니메이션 종료 후 알림
		if (OnFadeFinishedDelegate.IsBound())
		{
			OnFadeFinishedDelegate.Broadcast();
		}
	}
}
