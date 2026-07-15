// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/Widget.h"

#include "IndicatorLayer.generated.h"

class UECIndicatorManagerComponent;
class SActorCanvas;
class SWidget;
class UObject;

/**
 * Indicator 위젯을 화면에 배치하는 SActorCanvas를 UMG에서 사용할 수 있도록 제공합니다.
 */
UCLASS()
class UIndicatorLayer : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	/** 화면 가장자리에 고정된 인디케이터의 방향을 표시할 화살표 브러시를 설정합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	FSlateBrush ArrowBrush;

protected:
	// UWidget interface

	/** UWidget이 보유한 Slate 리소스를 해제합니다. */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	
	/** 인디케이터를 배치할 SActorCanvas를 생성합니다. */
	virtual TSharedRef<SWidget> RebuildWidget() override;
	
	// End UWidget

protected:
	/** 인디케이터의 위치 계산과 실제 화면 배치를 담당하는 Slate 위젯을 보관합니다. */
	TSharedPtr<SActorCanvas> MyActorCanvas;
};
