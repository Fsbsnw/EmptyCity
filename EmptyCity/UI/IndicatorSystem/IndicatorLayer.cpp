// Copyright Epic Games, Inc. All Rights Reserved.

#include "IndicatorLayer.h"

#include "SActorCanvas.h"
#include "Widgets/Layout/SBox.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IndicatorLayer)

class SWidget;

/////////////////////////////////////////////////////
// UIndicatorLayer

UIndicatorLayer::UIndicatorLayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 블루프린트에서 이 위젯을 변수로 참조할 수 있도록 설정합니다.
	bIsVariable = true;

	// 인디케이터는 마우스 입력을 막지 않도록 설정합니다.
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UIndicatorLayer::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	// SActorCanvas의 참조를 해제하여 내부 Slate 리소스와 위젯 풀이 정리될 수 있도록 합니다.
	MyActorCanvas.Reset();
}

TSharedRef<SWidget> UIndicatorLayer::RebuildWidget()
{
	if (!IsDesignTime())
	{
		ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
		if (ensureMsgf(LocalPlayer, TEXT("Attempting to rebuild a UActorCanvas without a valid LocalPlayer!")))
		{
			// 로컬 플레이어의 화면을 기준으로 인디케이터를 배치할 ActorCanvas를 생성합니다.
			MyActorCanvas = SNew(SActorCanvas, FLocalPlayerContext(LocalPlayer), &ArrowBrush);
			return MyActorCanvas.ToSharedRef();
		}
	}

	// Give it a trivial box, NullWidget isn't safe to use from a UWidget
	return SNew(SBox);
}