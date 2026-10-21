// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECPannableMapWidget.generated.h"

class UECMapNodeInfoWidget;
class UECMapNodeWidget;
class UECRegionVisitLogWidget;
class UCanvasPanel;
class UVerticalBox;

/**
 * 확대 및 위치 이동이 가능한 맵 위젯입니다.
 * 맵 노드를 선택하면 해당 위치로 확대하며 관련 정보를 표시합니다.
 */
UCLASS()
class EMPTYCITY_API UECPannableMapWidget : public UECUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	/** 실제 화면에 표시되는 고정 영역입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel_Root;

	/** 확대된 맵을 잘라내는 고정 표시 영역입니다. 기존 맵 위젯과의 호환을 위해 선택적으로 바인딩합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCanvasPanel> CanvasPanel_MapViewport;

	/** 맵 이미지와 모든 맵 노드를 포함하는 확대·이동 대상입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel_MapContent;

	/** 맵 노드 정보를 표시하는 위젯입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UECMapNodeInfoWidget* MapNodeInfoWidget;

	/** 최근 방문 로그 엔트리가 추가될 컨테이너입니다. 레이아웃과 스타일은 WBP에서 구성합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> VerticalBox_VisitHistory;

	/** 방문 로그 한 건을 표시할 WBP 클래스입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "방문 기록")
	TSubclassOf<UECRegionVisitLogWidget> VisitHistoryEntryWidgetClass;

	/** 맵에 표시할 최근 방문 기록의 최대 개수입니다. */
	UPROPERTY(EditAnywhere, Category = "방문 기록", meta = (ClampMin = "1", ClampMax = "20"))
	int32 MaxVisibleVisitHistoryEntries = 5;

	/** 맵 노드 정보창과 화면 가장자리 사이의 간격입니다. */
	UPROPERTY(EditAnywhere, Category = "맵 노드 정보", meta = (ClampMin = "0.0"))
	float MapNodeInfoSideMargin = 80.f;

	/** 맵 노드를 선택했을 때 적용할 확대 배율입니다. */
	UPROPERTY(EditAnywhere, Category = "맵 확대", meta = (ClampMin = "0.01", ClampMax = "3.0"))
	float TargetZoomInScale = 2.f;

	/** 맵에 적용할 수 있는 최대 확대 배율입니다. */
	UPROPERTY(EditAnywhere, Category = "맵 확대", meta = (ClampMin = "0.01", ClampMax = "3.0"))
	float MaxZoomScale = 3.f;

	/** 맵 포커스 이동 애니메이션의 재생 시간입니다. */
	UPROPERTY(EditAnywhere, Category = "맵 확대", meta = (ClampMin = "0.01"))
	float FocusAnimDuration = 0.35f;
	
private:
	/** 지역 서브시스템의 방문 기록을 최신순으로 표시합니다. */
	void RefreshVisitHistory();

	/** 방문 기록이 변경되었을 때 표시를 다시 구성합니다. */
	void HandleVisitHistoryChanged();

	/** MapViewport가 없는 기존 WBP에는 실행 시 고정 클리핑 영역을 생성합니다. */
	void EnsureMapViewport();

	/** 맵 정보창이 확대되지 않으면서 MapViewport 내부 좌우에 배치되도록 부모를 정리합니다. */
	void EnsureMapNodeInfoInViewport();

	/** 클릭한 맵 노드를 처리합니다. */
	UFUNCTION()
	void HandleMapNodeClicked(UECMapNodeWidget* MapNodeWidget);

	/** 지정한 맵 로컬 좌표가 화면 중앙에 오도록 확대 및 이동합니다. */
	void FocusMapAtLocalPosition(const FVector2D& MapLocalPosition, float TargetScale);

	/** 맵 노드의 중심 좌표를 MapContent 기준으로 계산합니다. */
	FVector2D GetMapNodeCenter(const UECMapNodeWidget* MapNodeWidget) const;

	/** 현재 트랜스폼에서 목표 트랜스폼으로 이동하는 애니메이션을 시작합니다. */
	void StartMapTransformAnimation(const FWidgetTransform& TargetTransform);

	/** 맵 콘텐츠가 화면 밖으로 지나치게 벗어나지 않도록 이동 범위를 제한합니다. */
	FWidgetTransform GetClampedMapTransform(FWidgetTransform Transform) const;

	/** 맵을 최초 위치와 배율로 되돌립니다. */
	void ResetMapScale();

	/** 선택한 맵 노드의 정보창을 표시합니다. */
	void ShowMapNodeInfo(UECMapNodeWidget* MapNodeWidget, const FVector2D& MapLocalPosition);

	/** 선택한 노드의 위치에 따라 정보창의 표시 방향을 설정합니다. */
	void SetMapNodeInfoSide(bool bShowOnLeft);

	/** 현재 선택된 맵 노드의 지역 태그입니다. */
	FGameplayTag SelectedRegionTag;

	bool bIsFocusAnimating = false;
	float FocusAnimElapsed = 0.f;

	FWidgetTransform InitialMapTransform;
	FWidgetTransform FocusStartTransform;
	FWidgetTransform FocusTargetTransform;
};
