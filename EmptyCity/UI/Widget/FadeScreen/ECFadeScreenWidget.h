// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECFadeScreenWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnFadeFinishedDelegate);

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECFadeScreenWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	/** 위젯이 화면에 띄워질 때 자동으로 애니메이션을 재생시키는 함수입니다. */
	void PlayTransition(float AnimSpeed = 1.f);

	/** 페이드 효과가 완료된 후에 호출되는 델리게이트입니다. */
	FOnFadeFinishedDelegate OnFadeFinishedDelegate;
	
protected:
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

	/** 블루프린트(WBP)에서 만든 동일한 이름의 애니메이션 타임라인을 C++로 가져옵니다. */
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeAnim;
};
