// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Map/ECPannableMapWidget.h"

#include "ECMapNodeInfoWidget.h"
#include "ECMapNodeWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UECPannableMapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CanvasPanel_MapContent)
	{
		InitialMapTransform = CanvasPanel_MapContent->GetRenderTransform();
	}
	
	if (!WidgetTree)
	{
		return;
	}

	AllMapNodes.Reset();

	// WBP에 배치된 맵 노드를 찾아 태그별로 캐싱합니다.
	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		UECMapNodeWidget* MapNode = Cast<UECMapNodeWidget>(Widget);
		if (!MapNode || !MapNode->MyNodeTag.IsValid())
		{
			return;
		}

		AllMapNodes.Add(MapNode->MyNodeTag, MapNode);

		MapNode->OnNodeClicked.RemoveAll(this);
		MapNode->OnNodeClicked.AddUObject(this, &ThisClass::HandleMapNodeClicked);
	});
}

FReply UECPannableMapWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	ResetMapScale();

	if (MapNodeInfoWidget)
	{
		MapNodeInfoWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return FReply::Handled();
}

void UECPannableMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsFocusAnimating || !CanvasPanel_MapContent)
	{
		return;
	}

	FocusAnimElapsed += InDeltaTime;
	
	const float Alpha = FocusAnimDuration <= KINDA_SMALL_NUMBER	? 1.f : FMath::Clamp(FocusAnimElapsed / FocusAnimDuration, 0.f, 1.f);
	const float EaseAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f);

	FWidgetTransform NewTransform;
	NewTransform.Translation = FMath::Lerp(
		FocusStartTransform.Translation,
		FocusTargetTransform.Translation,
		EaseAlpha
	);
	NewTransform.Scale = FMath::Lerp(
		FocusStartTransform.Scale,
		FocusTargetTransform.Scale,
		EaseAlpha
	);
	NewTransform.Shear = FMath::Lerp(
		FocusStartTransform.Shear,
		FocusTargetTransform.Shear,
		EaseAlpha
	);
	NewTransform.Angle = FMath::Lerp(
		FocusStartTransform.Angle,
		FocusTargetTransform.Angle,
		EaseAlpha
	);

	CanvasPanel_MapContent->SetRenderTransform(NewTransform);

	if (Alpha >= 1.f)
	{
		bIsFocusAnimating = false;
		CanvasPanel_MapContent->SetRenderTransform(FocusTargetTransform);
	}
}

void UECPannableMapWidget::HandleMapNodeClicked(UECMapNodeWidget* MapNodeWidget)
{
	FVector2D MapLocalPosition = GetMapNodeCenter(MapNodeWidget);

	FocusMapAtLocalPosition(MapLocalPosition, TargetZoomInScale);
	ShowMapNodeInfo(MapNodeWidget, MapLocalPosition);
}

void UECPannableMapWidget::FocusMapAtLocalPosition(const FVector2D& MapLocalPosition, float TargetScale)
{
	if (!CanvasPanel_Root || !CanvasPanel_MapContent)
	{
		return;
	}

	const FVector2D ViewSize = CanvasPanel_Root->GetCachedGeometry().GetLocalSize();
	const FVector2D MapSize = CanvasPanel_MapContent->GetCachedGeometry().GetLocalSize();
	
	if (MapSize.X <= KINDA_SMALL_NUMBER || MapSize.Y <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	// 축소했을 때 맵 바깥의 빈 영역이 드러나지 않는 최소 배율입니다.
	const float MinScale = FMath::Max(ViewSize.X / MapSize.X, ViewSize.Y / MapSize.Y);
	const float ClampedScale = FMath::Clamp(TargetScale, MinScale, MaxZoomScale);

	FWidgetTransform TargetTransform = CanvasPanel_MapContent->GetRenderTransform();
	TargetTransform.Scale = FVector2D(ClampedScale, ClampedScale);

	const FVector2D MapCenter = MapSize * 0.5f;
	// 선택한 맵 좌표가 고정 화면의 중앙에 오도록 이동합니다.
	TargetTransform.Translation = -(MapLocalPosition - MapCenter) * ClampedScale;

	StartMapTransformAnimation(GetClampedMapTransform(TargetTransform));
}

FVector2D UECPannableMapWidget::GetMapNodeCenter(const UECMapNodeWidget* MapNodeWidget) const
{
	if (!MapNodeWidget || !CanvasPanel_MapContent)
	{
		return FVector2D::ZeroVector;
	}

	const UCanvasPanelSlot* NodeSlot = Cast<UCanvasPanelSlot>(MapNodeWidget->Slot);
	if (!NodeSlot)
	{
		return FVector2D::ZeroVector;
	}

	const FVector2D MapSize = CanvasPanel_MapContent->GetCachedGeometry().GetLocalSize();
	const FAnchors Anchors = NodeSlot->GetAnchors();

	// 맵 노드는 Stretch가 아닌 단일 Anchor 사용을 전제로 합니다.
	const FVector2D AnchorPosition(MapSize.X * Anchors.Minimum.X,MapSize.Y * Anchors.Minimum.Y);
	const FVector2D NodeSize = MapNodeWidget->GetCachedGeometry().GetLocalSize();
	const FVector2D Alignment =	NodeSlot->GetAlignment();
	
	return AnchorPosition + NodeSlot->GetPosition() + (FVector2D(0.5f, 0.5f) - Alignment) * NodeSize;
}

void UECPannableMapWidget::StartMapTransformAnimation(const FWidgetTransform& TargetTransform)
{
	if (!CanvasPanel_MapContent)
	{
		return;
	}

	FocusStartTransform = CanvasPanel_MapContent->GetRenderTransform();
	FocusTargetTransform = TargetTransform;
	FocusAnimElapsed = 0.f;
	bIsFocusAnimating = true;
}

FWidgetTransform UECPannableMapWidget::GetClampedMapTransform(FWidgetTransform Transform) const
{
	if (!CanvasPanel_Root || !CanvasPanel_MapContent)
	{
		return Transform;
	}

	const FVector2D CanvasSize = CanvasPanel_Root->GetCachedGeometry().GetLocalSize();
	const FVector2D MapSize = CanvasPanel_MapContent->GetCachedGeometry().GetLocalSize();

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

void UECPannableMapWidget::ResetMapScale()
{
	StartMapTransformAnimation(InitialMapTransform);
}

void UECPannableMapWidget::ShowMapNodeInfo(UECMapNodeWidget* MapNodeWidget, const FVector2D& MapLocalPosition)
{
	if (!MapNodeWidget || !CanvasPanel_MapContent || !MapNodeInfoWidget)
	{
		return;
	}

	const FVector2D MapSize = CanvasPanel_MapContent->GetCachedGeometry().GetLocalSize();
	// 오른쪽 노드를 눌렀다면 정보창은 왼쪽에 표시합니다.
	const bool bShowOnLeft = MapLocalPosition.X > MapSize.X * 0.5f;

	SetMapNodeInfoSide(bShowOnLeft);
	// @TODO: 맵 지역의 이름 데이터 변경하기
	MapNodeInfoWidget->SetMapName(FText::FromName(MapNodeWidget->MyNodeTag.GetTagName()));
	MapNodeInfoWidget->SetVisibility(ESlateVisibility::Visible);
}

void UECPannableMapWidget::SetMapNodeInfoSide(bool bShowOnLeft)
{
	if (!MapNodeInfoWidget)
	{
		return;
	}

	UCanvasPanelSlot* InfoSlot = Cast<UCanvasPanelSlot>(MapNodeInfoWidget->Slot);
	if (!InfoSlot)
	{
		return;
	}

	if (bShowOnLeft)
	{
		InfoSlot->SetAnchors(FAnchors(0.f, 0.5f));
		InfoSlot->SetAlignment(FVector2D(0.f, 0.5f));
		InfoSlot->SetPosition(FVector2D(MapNodeInfoSideMargin,0.f));
	}
	else
	{
		InfoSlot->SetAnchors(FAnchors(1.f, 0.5f));
		InfoSlot->SetAlignment(FVector2D(1.f, 0.5f));
		InfoSlot->SetPosition(FVector2D(-MapNodeInfoSideMargin,0.f));
	}
}