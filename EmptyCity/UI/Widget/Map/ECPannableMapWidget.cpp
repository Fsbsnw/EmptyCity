// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Map/ECPannableMapWidget.h"

#include "ECGameplayTags.h"
#include "ECMapNodeInfoWidget.h"
#include "ECMapNodeWidget.h"
#include "ECRegionVisitLogWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/VerticalBox.h"
#include "Data/Region/ECRegionTableRow.h"
#include "Engine/GameInstance.h"
#include "Subsystem/ECRegionSubsystem.h"
#include "Subsystem/ECSoundSubsystem.h"
#include "UI/Subsystem/UIManagerSubsystem.h"

namespace MapSoundRows
{
	const FName TravelSuccess(TEXT("UI_Map_Travel_Success"));
	const FName TravelFail(TEXT("UI_Map_Travel_Fail"));
}

void UECPannableMapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureMapViewport();
	EnsureMapNodeInfoInViewport();

	UECRegionSubsystem& RegionSubsystem = UECRegionSubsystem::Get(this);
	RegionSubsystem.OnRegionVisitHistoryChanged.RemoveAll(this);
	RegionSubsystem.OnRegionVisitHistoryChanged.AddUObject(this, &ThisClass::HandleVisitHistoryChanged);
	RefreshVisitHistory();

	if (CanvasPanel_MapContent)
	{
		InitialMapTransform = CanvasPanel_MapContent->GetRenderTransform();
	}
	
	if (!WidgetTree)
	{
		return;
	}

	// WBP에 배치된 맵 노드의 클릭 이벤트를 연결합니다.
	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		UECMapNodeWidget* MapNode = Cast<UECMapNodeWidget>(Widget);
		if (!MapNode || !MapNode->MyNodeTag.IsValid())
		{
			return;
		}

		MapNode->OnNodeClicked.RemoveAll(this);
		MapNode->OnNodeClicked.AddUObject(this, &ThisClass::HandleMapNodeClicked);
	});
}

void UECPannableMapWidget::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UECRegionSubsystem* RegionSubsystem = GameInstance->GetSubsystem<UECRegionSubsystem>())
		{
			RegionSubsystem->OnRegionVisitHistoryChanged.RemoveAll(this);
		}
	}

	Super::NativeDestruct();
}

void UECPannableMapWidget::RefreshVisitHistory()
{
	if (!VerticalBox_VisitHistory || !VisitHistoryEntryWidgetClass)
	{
		return;
	}

	VerticalBox_VisitHistory->ClearChildren();

	const UECRegionSubsystem& RegionSubsystem = UECRegionSubsystem::Get(this);
	const TArray<FECRegionVisitRecord>& VisitHistory = RegionSubsystem.GetVisitHistory();
	const int32 VisibleEntryCount = FMath::Min(MaxVisibleVisitHistoryEntries, VisitHistory.Num());
	for (int32 VisibleIndex = 0; VisibleIndex < VisibleEntryCount; ++VisibleIndex)
	{
		const FECRegionVisitRecord& Record = VisitHistory[VisitHistory.Num() - 1 - VisibleIndex];
		const FECRegionTableRow* RegionDefinition = RegionSubsystem.FindRegionDefinition(Record.RegionTag);
		if (!RegionDefinition)
		{
			continue;
		}

		UECRegionVisitLogWidget* VisitLogWidget = CreateWidget<UECRegionVisitLogWidget>(
			GetOwningPlayer(),
			VisitHistoryEntryWidgetClass
		);
		if (!VisitLogWidget)
		{
			continue;
		}

		VisitLogWidget->SetVisitRecord(Record, RegionDefinition->RegionVisitHistoryImage);
		VerticalBox_VisitHistory->AddChildToVerticalBox(VisitLogWidget);
	}
}

void UECPannableMapWidget::HandleVisitHistoryChanged()
{
	RefreshVisitHistory();
}

void UECPannableMapWidget::EnsureMapViewport()
{
	if (!WidgetTree || !CanvasPanel_MapContent)
	{
		return;
	}

	if (!CanvasPanel_MapViewport)
	{
		CanvasPanel_MapViewport = Cast<UCanvasPanel>(
			WidgetTree->FindWidget(TEXT("CanvasPanel_MapViewport"))
		);
	}

	if (CanvasPanel_MapViewport)
	{
		CanvasPanel_MapViewport->SetClipping(EWidgetClipping::ClipToBoundsAlways);
		CanvasPanel_MapContent->SetClipping(EWidgetClipping::Inherit);
		return;
	}

	UCanvasPanel* ParentCanvas = Cast<UCanvasPanel>(CanvasPanel_MapContent->GetParent());
	UCanvasPanelSlot* OldContentSlot = Cast<UCanvasPanelSlot>(CanvasPanel_MapContent->Slot);
	if (!ParentCanvas || !OldContentSlot)
	{
		return;
	}

	const int32 ContentIndex = ParentCanvas->GetChildIndex(CanvasPanel_MapContent);
	if (ContentIndex == INDEX_NONE)
	{
		return;
	}

	const FAnchorData OldLayout = OldContentSlot->GetLayout();
	const bool bOldAutoSize = OldContentSlot->GetAutoSize();
	const int32 OldZOrder = OldContentSlot->GetZOrder();
	TArray<UCanvasPanelSlot*> LaterSiblingSlotsAtSameZOrder;
	for (int32 SiblingIndex = ContentIndex + 1;
		SiblingIndex < ParentCanvas->GetChildrenCount();
		++SiblingIndex)
	{
		UWidget* Sibling = ParentCanvas->GetChildAt(SiblingIndex);
		UCanvasPanelSlot* SiblingSlot = Sibling
			? Cast<UCanvasPanelSlot>(Sibling->Slot)
			: nullptr;
		if (SiblingSlot && SiblingSlot->GetZOrder() == OldZOrder)
		{
			LaterSiblingSlotsAtSameZOrder.Add(SiblingSlot);
		}
	}

	if (!ParentCanvas->RemoveChild(CanvasPanel_MapContent))
	{
		return;
	}

	CanvasPanel_MapViewport = WidgetTree->ConstructWidget<UCanvasPanel>(
		UCanvasPanel::StaticClass(),
		TEXT("CanvasPanel_MapViewport")
	);
	CanvasPanel_MapViewport->SetClipping(EWidgetClipping::ClipToBoundsAlways);

	UCanvasPanelSlot* ViewportSlot = ParentCanvas->AddChildToCanvas(CanvasPanel_MapViewport);
	if (!ViewportSlot)
	{
		return;
	}

	ViewportSlot->SetLayout(OldLayout);
	ViewportSlot->SetAutoSize(bOldAutoSize);
	ViewportSlot->SetZOrder(OldZOrder);
	for (UCanvasPanelSlot* LaterSiblingSlot : LaterSiblingSlotsAtSameZOrder)
	{
		LaterSiblingSlot->SetZOrder(OldZOrder + 1);
	}

	UCanvasPanelSlot* ContentSlot = CanvasPanel_MapViewport->AddChildToCanvas(
		CanvasPanel_MapContent
	);
	ContentSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	ContentSlot->SetOffsets(FMargin(0.f));
	ContentSlot->SetAlignment(FVector2D::ZeroVector);
	ContentSlot->SetAutoSize(false);
	ContentSlot->SetZOrder(0);
	CanvasPanel_MapContent->SetClipping(EWidgetClipping::Inherit);
}

void UECPannableMapWidget::EnsureMapNodeInfoInViewport()
{
	if (!CanvasPanel_MapViewport || !MapNodeInfoWidget)
	{
		return;
	}

	if (MapNodeInfoWidget->GetParent() == CanvasPanel_MapViewport)
	{
		if (UCanvasPanelSlot* InfoSlot = Cast<UCanvasPanelSlot>(MapNodeInfoWidget->Slot))
		{
			InfoSlot->SetZOrder(FMath::Max(1, InfoSlot->GetZOrder()));
		}
		return;
	}

	UPanelWidget* OldParent = MapNodeInfoWidget->GetParent();
	if (!OldParent)
	{
		return;
	}

	FVector2D InfoSize = MapNodeInfoWidget->GetDesiredSize();
	bool bAutoSize = true;
	int32 InfoZOrder = 1;
	if (const UCanvasPanelSlot* OldInfoSlot = Cast<UCanvasPanelSlot>(MapNodeInfoWidget->Slot))
	{
		InfoSize = OldInfoSlot->GetSize();
		bAutoSize = OldInfoSlot->GetAutoSize();
		InfoZOrder = FMath::Max(1, OldInfoSlot->GetZOrder());
	}

	if (!OldParent->RemoveChild(MapNodeInfoWidget))
	{
		return;
	}

	UCanvasPanelSlot* InfoSlot = CanvasPanel_MapViewport->AddChildToCanvas(MapNodeInfoWidget);
	if (!InfoSlot)
	{
		return;
	}

	InfoSlot->SetAnchors(FAnchors(0.f, 0.5f));
	InfoSlot->SetAlignment(FVector2D(0.f, 0.5f));
	InfoSlot->SetPosition(FVector2D(MapNodeInfoSideMargin, 0.f));
	InfoSlot->SetAutoSize(bAutoSize);
	if (!bAutoSize && InfoSize.X > 0.f && InfoSize.Y > 0.f)
	{
		InfoSlot->SetSize(InfoSize);
	}
	InfoSlot->SetZOrder(InfoZOrder);
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

FReply UECPannableMapWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() != EKeys::E)
	{
		return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	}

	if (!SelectedRegionTag.IsValid())
	{
		return FReply::Handled();
	}

	UECRegionSubsystem& RegionSubsystem = UECRegionSubsystem::Get(this);
	if (!RegionSubsystem.RequestTravel(SelectedRegionTag))
	{
		UECSoundSubsystem::Get(this).PlaySFXByName(MapSoundRows::TravelFail);
		return FReply::Handled();
	}

	UECSoundSubsystem::Get(this).PlaySFXByName(MapSoundRows::TravelSuccess);

	if (APlayerController* PC = GetOwningPlayer())
	{
		UUIManagerSubsystem::Get(PC)->RequestToggleInput(ECGameplayTags::InputTag_UI_Map);
	}
	
	ResetMapScale();

	if (MapNodeInfoWidget)
	{
		MapNodeInfoWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return FReply::Handled();
}

void UECPannableMapWidget::HandleMapNodeClicked(UECMapNodeWidget* MapNodeWidget)
{
	if (!IsValid(MapNodeWidget))
	{
		return;
	}

	SelectedRegionTag = MapNodeWidget->MyNodeTag;

	FVector2D MapLocalPosition = GetMapNodeCenter(MapNodeWidget);

	FocusMapAtLocalPosition(MapLocalPosition, TargetZoomInScale);
	ShowMapNodeInfo(MapNodeWidget, MapLocalPosition);
}

void UECPannableMapWidget::FocusMapAtLocalPosition(const FVector2D& MapLocalPosition, float TargetScale)
{
	if (!CanvasPanel_MapContent)
	{
		return;
	}

	const UCanvasPanel* MapViewport = CanvasPanel_MapViewport
		? CanvasPanel_MapViewport.Get()
		: CanvasPanel_Root.Get();
	if (!MapViewport)
	{
		return;
	}

	const FVector2D ViewSize = MapViewport->GetCachedGeometry().GetLocalSize();
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

	const FGeometry& MapGeometry =
		CanvasPanel_MapContent->GetCachedGeometry();
	const FGeometry& NodeGeometry =
		MapNodeWidget->GetCachedGeometry();

	const FVector2D NodeCenterAbsolute =
		NodeGeometry.LocalToAbsolute(
			NodeGeometry.GetLocalSize() * 0.5f);

	return MapGeometry.AbsoluteToLocal(NodeCenterAbsolute);
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
	if (!CanvasPanel_MapContent)
	{
		return Transform;
	}

	const UCanvasPanel* MapViewport = CanvasPanel_MapViewport
		? CanvasPanel_MapViewport.Get()
		: CanvasPanel_Root.Get();
	if (!MapViewport)
	{
		return Transform;
	}

	const FVector2D CanvasSize = MapViewport->GetCachedGeometry().GetLocalSize();
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

	bool bShowOnLeft = false;
	if (CanvasPanel_MapViewport)
	{
		const FGeometry& NodeGeometry = MapNodeWidget->GetCachedGeometry();
		const FVector2D NodeCenterAbsolute = NodeGeometry.LocalToAbsolute(
			NodeGeometry.GetLocalSize() * 0.5f
		);
		const FGeometry& ViewportGeometry = CanvasPanel_MapViewport->GetCachedGeometry();
		const FVector2D NodeCenterInViewport = ViewportGeometry.AbsoluteToLocal(
			NodeCenterAbsolute
		);
		bShowOnLeft = NodeCenterInViewport.X > ViewportGeometry.GetLocalSize().X * 0.5f;
	}
	else
	{
		const FVector2D MapSize = CanvasPanel_MapContent->GetCachedGeometry().GetLocalSize();
		bShowOnLeft = MapLocalPosition.X > MapSize.X * 0.5f;
	}

	// Viewport 오른쪽에 보이는 노드를 눌렀다면 정보창은 왼쪽에 표시합니다.

	SetMapNodeInfoSide(bShowOnLeft);

	const FECRegionTableRow* RegionDefinition =
		UECRegionSubsystem::Get(this).FindRegionDefinition(MapNodeWidget->MyNodeTag);
	if (RegionDefinition)
	{
		MapNodeInfoWidget->SetMapNodeInfoImage(RegionDefinition->RegionInfoImage);
		MapNodeInfoWidget->SetRegionImage(RegionDefinition->RegionImage);
	}

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
