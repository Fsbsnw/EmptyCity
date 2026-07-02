// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Map/ECPannableMapWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"

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
