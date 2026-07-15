// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Map/ECPannableMapWidget.h"

#include "ECMapNodeWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"

void UECPannableMapWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 1. WBP 내부에 배치된 모든 하위 위젯을 순회하며 맵 노드만 찾아 캐싱합니다.
	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([this](UWidget* Widget)
		{
			// 위젯이 맵 노드 클래스인지 확인
			if (UECMapNodeWidget* MapNode = Cast<UECMapNodeWidget>(Widget))
			{
				// 태그가 유효하다면 TMap에 등록
				if (MapNode->MyNodeTag.IsValid())
				{
					AllMapNodes.Add(MapNode->MyNodeTag, MapNode);
					
					// Clicked 이벤트 바인딩
					MapNode->OnNodeClicked.AddUObject(this, &ThisClass::HandleMapNodeClicked);
				}
			}
		});
	}
}

FReply UECPannableMapWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const float WheelDelta = InMouseEvent.GetWheelDelta();
	const float ZoomStep = 0.02f;

	const FVector2D MouseScreenPosition = InMouseEvent.GetScreenSpacePosition();
	const FGeometry& CanvasGeometry = CanvasPanel_Root->GetCachedGeometry();

	const FVector2D MouseLocalPosition = CanvasGeometry.AbsoluteToLocal(MouseScreenPosition);
	const FVector2D CanvasCenterPosition = CanvasGeometry.GetLocalSize() * 0.5f;

	FWidgetTransform Transform = Image_Map->GetRenderTransform();
	
	const FVector2D ImageCenter = CanvasCenterPosition + Transform.Translation;
	const FVector2D MouseOffsetFromImageCenter = MouseLocalPosition - ImageCenter;

	const float MinScale = CanvasGeometry.GetLocalSize().X / Image_Map->GetDesiredSize().X;
	const float MaxScale = 3.f;
	
	const float OldScale = Transform.Scale.X;
	const float NewScale = FMath::Clamp(OldScale + WheelDelta * ZoomStep, MinScale, MaxScale);

	const float ScaleRatio = NewScale / OldScale;

	Transform.Scale = FVector2D(NewScale, NewScale);

	// 확대/축소된 비율만큼 이동 보정
	Transform.Translation -= MouseOffsetFromImageCenter * (ScaleRatio - 1.f);

	Image_Map->SetRenderTransform(Transform);
	ClampMapTransform();

	return FReply::Handled();
}

FReply UECPannableMapWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 드래그가 켜져 있고, 마우스 왼쪽 버튼을 눌렀다면 드래그 시작!
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;
		DragStartMousePosition = InMouseEvent.GetScreenSpacePosition();
		DragStartWidgetTranslation = Image_Map->GetRenderTransform().Translation;

		// 마우스가 위젯을 계속 캡처합니다.
		return FReply::Handled().CaptureMouse(TakeWidget());
	}

	return FReply::Handled();
}

FReply UECPannableMapWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDragging)
	{
		// 1. 마우스가 얼마나 이동했는지 계산
		FVector2D MouseDelta = InMouseEvent.GetScreenSpacePosition() - DragStartMousePosition;

		// 2. 현재 화면의 DPI 스케일로 나눠주어야 마우스와 1:1로 딱 붙어서 이동합니다.
		float DPIScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
		FVector2D NewTranslation = DragStartWidgetTranslation + (MouseDelta / DPIScale);

		// 3. 위젯의 렌더링 위치 업데이트
		Image_Map->SetRenderTranslation(NewTranslation);
		ClampMapTransform();

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply UECPannableMapWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDragging && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = false;
        
		// 마우스 캡처를 풀어줍니다.
		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Handled();
}

void UECPannableMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsFocusAnimating || !Image_Map)
	{
		return;
	}

	FocusAnimElapsed += InDeltaTime;

	const float Alpha = FocusAnimDuration <= KINDA_SMALL_NUMBER
		? 1.f
		: FMath::Clamp(FocusAnimElapsed / FocusAnimDuration, 0.f, 1.f);

	const float EaseAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f);

	FWidgetTransform NewTransform = FocusStartTransform;

	NewTransform.Scale = FMath::Lerp(
		FocusStartTransform.Scale,
		FocusTargetTransform.Scale,
		EaseAlpha
	);

	NewTransform.Translation = FMath::Lerp(
		FocusStartTransform.Translation,
		FocusTargetTransform.Translation,
		EaseAlpha
	);

	NewTransform = GetClampedMapTransform(NewTransform);

	Image_Map->SetRenderTransform(NewTransform);

	if (Alpha >= 1.f)
	{
		bIsFocusAnimating = false;
		Image_Map->SetRenderTransform(FocusTargetTransform);
	}
}

void UECPannableMapWidget::FocusMapAtLocalPosition(const FVector2D& MapLocalPosition, float TargetScale)
{
	if (!CanvasPanel_Root || !Image_Map)
	{
		return;
	}

	const FVector2D CanvasSize = CanvasPanel_Root->GetCachedGeometry().GetLocalSize();
	const FVector2D MapSize = Image_Map->GetDesiredSize();

	if (MapSize.IsNearlyZero())
	{
		return;
	}

	const float MinScale = CanvasSize.X / MapSize.X;
	const float MaxScale = 3.f;

	TargetScale = FMath::Clamp(TargetScale, MinScale, MaxScale);

	FocusStartTransform = Image_Map->GetRenderTransform();
	FocusTargetTransform = FocusStartTransform;

	FocusTargetTransform.Scale = FVector2D(TargetScale, TargetScale);

	const FVector2D MapCenter = MapSize * 0.5f;

	// MapLocalPosition이 화면 중앙에 오도록 Translation 계산
	FocusTargetTransform.Translation = -(MapLocalPosition - MapCenter) * TargetScale;

	// 가장자리 밖으로 못 나가게 보정
	FocusTargetTransform = GetClampedMapTransform(FocusTargetTransform);

	FocusAnimElapsed = 0.f;
	bIsFocusAnimating = true;
}

FWidgetTransform UECPannableMapWidget::GetClampedMapTransform(FWidgetTransform Transform) const
{
	if (!CanvasPanel_Root || !Image_Map)
	{
		return Transform;
	}

	const FVector2D CanvasSize = CanvasPanel_Root->GetCachedGeometry().GetLocalSize();
	const FVector2D MapSize = Image_Map->GetDesiredSize();

	const float ScaledWidth = MapSize.X * Transform.Scale.X;
	const float ScaledHeight = MapSize.Y * Transform.Scale.Y;

	const float MaxOffsetX = FMath::Max(0.f, (ScaledWidth - CanvasSize.X) * 0.5f);
	const float MaxOffsetY = FMath::Max(0.f, (ScaledHeight - CanvasSize.Y) * 0.5f);

	Transform.Translation.X = FMath::Clamp(
		Transform.Translation.X,
		-MaxOffsetX,
		MaxOffsetX
	);

	Transform.Translation.Y = FMath::Clamp(
		Transform.Translation.Y,
		-MaxOffsetY,
		MaxOffsetY
	);

	return Transform;
}

void UECPannableMapWidget::HandleMapNodeClicked(UECMapNodeWidget* MapNodeWidget)
{
	if (!MapNodeWidget)
	{
		return;
	}

	const FVector2D MapLocalPosition = MapNodeWidget->GetMapLocalPosition();

	FocusMapAtLocalPosition(MapLocalPosition, TargetZoomInScale);
}


void UECPannableMapWidget::ClampMapTransform()
{
	FWidgetTransform Transform = Image_Map->GetRenderTransform();

	const FVector2D CanvasSize= CanvasPanel_Root->GetCachedGeometry().GetLocalSize();
	const FVector2D MapSize = Image_Map->GetDesiredSize();

	const float ScaledWidth  = MapSize.X * Transform.Scale.X;
	const float ScaledHeight = MapSize.Y * Transform.Scale.Y;

	const float MaxOffsetX = FMath::Max(0.f, (ScaledWidth - CanvasSize.X) * 0.5f);
	const float MaxOffsetY = FMath::Max(0.f, (ScaledHeight - CanvasSize.Y) * 0.5f);

	Transform.Translation.X =
		FMath::Clamp(Transform.Translation.X,
					 -MaxOffsetX,
					  MaxOffsetX);

	Transform.Translation.Y =
		FMath::Clamp(Transform.Translation.Y,
					 -MaxOffsetY,
					  MaxOffsetY);

	Image_Map->SetRenderTransform(Transform);
}
