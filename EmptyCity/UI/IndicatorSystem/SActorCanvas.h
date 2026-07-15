// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidgetPool.h"
#include "Widgets/SPanel.h"

class FActiveTimerHandle;
class FArrangedChildren;
class FChildren;
class FPaintArgs;
class FReferenceCollector;
class FSlateRect;
class FSlateWindowElementList;
class FWidgetStyle;
class UIndicatorDescriptor;
class UECIndicatorManagerComponent;
struct FSlateBrush;


/**
 * 액터의 월드 위치를 화면 좌표로 변환하여 인디케이터 위젯을 배치하는 Slate 패널입니다.
 */
class SActorCanvas : public SPanel, public FGCObject
{
public:
	/** 개별 인디케이터 위젯의 배치 정보와 상태를 관리하는 슬롯입니다. */
	class FSlot : public TSlotBase<FSlot>
	{
	public: 

		FSlot(UIndicatorDescriptor* InIndicator)
			: TSlotBase<FSlot>()
			, Indicator(InIndicator)
			, ScreenPosition(FVector2D::ZeroVector)
			, Depth(0)
			, Priority(0.f)
			, bIsIndicatorVisible(true)
			, bInFrontOfCamera(true)
			, bHasValidScreenPosition(false)
			, bDirty(true)
			, bWasIndicatorClamped(false)
			, bWasIndicatorClampedStatusChanged(false)
		{
		}

		SLATE_SLOT_BEGIN_ARGS(FSlot, TSlotBase<FSlot>)
		SLATE_SLOT_END_ARGS()
		using TSlotBase<FSlot>::Construct;

		bool GetIsIndicatorVisible() const { return bIsIndicatorVisible; }
		void SetIsIndicatorVisible(bool bVisible)
		{
			if (bIsIndicatorVisible != bVisible)
			{
				bIsIndicatorVisible = bVisible;
				bDirty = true;
			}

			RefreshVisibility();
		}

		FVector2D GetScreenPosition() const { return ScreenPosition; }
		void SetScreenPosition(FVector2D InScreenPosition)
		{
			if (ScreenPosition != InScreenPosition)
			{
				ScreenPosition = InScreenPosition;
				bDirty = true;
			}
		}

		double GetDepth() const { return Depth; }
		void SetDepth(double InDepth)
		{
			if (Depth != InDepth)
			{
				Depth = InDepth;
				bDirty = true;
			}
		}

		int32 GetPriority() const { return Priority; }
		void SetPriority(int32 InPriority)
		{
			if (Priority != InPriority)
			{
				Priority = InPriority;
				bDirty = true;
			}
		}

		bool GetInFrontOfCamera() const { return bInFrontOfCamera; }
		void SetInFrontOfCamera(bool bInFront)
		{
			if (bInFrontOfCamera != bInFront)
			{
				bInFrontOfCamera = bInFront;
				bDirty = true;
			}

			RefreshVisibility();
		}

		bool HasValidScreenPosition() const { return bHasValidScreenPosition; }
		void SetHasValidScreenPosition(bool bValidScreenPosition)
		{
			if (bHasValidScreenPosition != bValidScreenPosition)
			{
				bHasValidScreenPosition = bValidScreenPosition;
				bDirty = true;
			}

			RefreshVisibility();
		}

		/** 슬롯의 배치 정보가 변경되었는지 반환합니다. */
		bool bIsDirty() const { return bDirty; }

		void ClearDirtyFlag()
		{
			bDirty = false;
		}

		/** 이전 프레임에 인디케이터가 화면 영역 안으로 제한되었는지 반환합니다. */
		bool WasIndicatorClamped() const { return bWasIndicatorClamped; }
		
		/** 인디케이터의 화면 제한 상태를 저장합니다. */
		void SetWasIndicatorClamped(bool bWasClamped) const
		{
			if (bWasClamped != bWasIndicatorClamped)
			{
				bWasIndicatorClamped = bWasClamped;
				bWasIndicatorClampedStatusChanged = true;
			}
		}

		/** 인디케이터의 화면 제한 상태가 변경되었는지 반환합니다. */
		bool WasIndicatorClampedStatusChanged() const { return bWasIndicatorClampedStatusChanged; }

		/** 인디케이터의 화면 제한 상태 변경 플래그를 초기화합니다. */
		void ClearIndicatorClampedStatusChangedFlag()
		{
			bWasIndicatorClampedStatusChanged = false;
		}

	private:
		void RefreshVisibility()
		{
			const bool bIsVisible = bIsIndicatorVisible && bHasValidScreenPosition;
			GetWidget()->SetVisibility(bIsVisible ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed);
		}

		/** 이 슬롯이 표시하는 인디케이터 정보입니다. */
		UIndicatorDescriptor* Indicator;
		FVector2D ScreenPosition;
		double Depth;
		int32 Priority;

		uint8 bIsIndicatorVisible : 1;
		uint8 bInFrontOfCamera : 1;
		uint8 bHasValidScreenPosition : 1;
		uint8 bDirty : 1;
		
		/** 이전 프레임에 인디케이터가 화면 영역 안으로 제한되었는지 저장합니다. */
		mutable uint8 bWasIndicatorClamped : 1;
		/** 인디케이터의 화면 제한 상태가 변경되었는지 나타냅니다. */
		mutable uint8 bWasIndicatorClampedStatusChanged : 1;

		friend class SActorCanvas;
	};

	/** 화면 밖 대상의 방향 화살표 위젯을 관리하는 슬롯입니다. */
	class FArrowSlot : public TSlotBase<FArrowSlot>
	{
	};

	/** Slate 위젯 생성 인자를 정의합니다. */
	SLATE_BEGIN_ARGS(SActorCanvas) {
		_Visibility = EVisibility::HitTestInvisible;
	}

	/** 캔버스가 관리할 인디케이터 슬롯을 전달받습니다. */
	SLATE_SLOT_ARGUMENT(SActorCanvas::FSlot, Slots)
		
	/** Slate 위젯 생성 인자 정의를 종료합니다. */
	SLATE_END_ARGS()

	/** 인디케이터와 화살표 자식 패널을 초기화합니다. */
	SActorCanvas()
		: CanvasChildren(this)
		, ArrowChildren(this)
		, AllChildren(this)
	{
		AllChildren.AddChildren(CanvasChildren);
		AllChildren.AddChildren(ArrowChildren);
	}

	void Construct(const FArguments& InArgs, const FLocalPlayerContext& InCtx, const FSlateBrush* ActorCanvasArrowBrush);

	// SWidget Interface
	
	/** 계산된 화면 좌표를 기준으로 인디케이터 자식 위젯을 배치합니다. */
	virtual void OnArrangeChildren( const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren ) const override;

	/** 별도의 고정 크기를 요구하지 않으므로 0 크기를 반환합니다. */
	virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }

	/** 캔버스가 관리하는 모든 자식 위젯을 반환합니다. */
	virtual FChildren* GetChildren() override { return &AllChildren; }
	
	/** 인디케이터와 화면 방향 화살표를 그립니다. */
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const;

	// End SWidget
	
	/** 인디케이터를 슬롯에 추가된 순서대로 그릴지 설정합니다. */
	void SetDrawElementsInOrder(bool bInDrawElementsInOrder) { bDrawElementsInOrder = bInDrawElementsInOrder; }

	/** 가비지 컬렉터에 표시할 참조자 이름을 반환합니다. */
	virtual FString GetReferencerName() const override;

	/** 캔버스가 참조하는 UObject를 가비지 컬렉터에 등록합니다. */
	virtual void AddReferencedObjects( FReferenceCollector& Collector ) override;
	
private:
	void OnIndicatorAdded(UIndicatorDescriptor* Indicator);
	void OnIndicatorRemoved(UIndicatorDescriptor* Indicator);

	/** 인디케이터 정보에 맞는 위젯과 슬롯을 생성합니다. */
	void AddIndicatorForEntry(UIndicatorDescriptor* Indicator);

	/** 인디케이터에 연결된 위젯과 슬롯을 제거합니다. */
	void RemoveIndicatorForEntry(UIndicatorDescriptor* Indicator);

	using FScopedWidgetSlotArguments = TPanelChildren<FSlot>::FScopedWidgetSlotArguments;
	
	/** 인디케이터를 표시할 새로운 액터 슬롯을 추가합니다. */
	FScopedWidgetSlotArguments AddActorSlot(UIndicatorDescriptor* Indicator);
	
	/** 지정된 위젯을 포함하는 액터 슬롯을 제거합니다. */
	int32 RemoveActorSlot(const TSharedRef<SWidget>& SlotWidget);

	/** 하나 이상의 인디케이터를 표시할 수 있는 상태인지 설정합니다. */
	void SetShowAnyIndicators(bool bIndicators);

	/** 인디케이터의 화면 위치와 표시 상태를 주기적으로 갱신합니다. */
	EActiveTimerReturnType UpdateCanvas(double InCurrentTime, float InDeltaTime);

	/** 인디케이터 위젯의 크기, 정렬 오프셋 및 화면 여백을 계산합니다. */
	void GetOffsetAndSize(const UIndicatorDescriptor* Indicator,
		FVector2D& OutSize, 
		FVector2D& OutOffset,
		FVector2D& OutPaddingMin,
		FVector2D& OutPaddingMax) const;

	/** 인디케이터 상태에 따라 활성 타이머를 등록하거나 해제합니다. */
	void UpdateActiveTimer();

private:
	/** 현재 캔버스가 관리하는 모든 인디케이터입니다. */
	TArray<TObjectPtr<UIndicatorDescriptor>> AllIndicators;

	/** 현재 표시되지 않아 갱신 대상에서 제외된 인디케이터입니다. */
	TArray<UIndicatorDescriptor*> InactiveIndicators;
	
	/** 화면 좌표 계산에 사용하는 로컬 플레이어 정보입니다. */
	FLocalPlayerContext LocalPlayerContext;

	/** 인디케이터의 추가 및 제거를 관리하는 컴포넌트입니다. */
	TWeakObjectPtr<UECIndicatorManagerComponent> IndicatorComponentPtr;

	/** 인디케이터 위젯을 관리하는 슬롯 목록입니다. */
	TPanelChildren<FSlot> CanvasChildren;

	/** 화면 밖 대상의 방향 화살표 위젯을 관리하는 슬롯 목록입니다. */
	mutable TPanelChildren<FArrowSlot> ArrowChildren;

	/** 인디케이터와 방향 화살표를 함께 관리하는 자식 목록입니다. */
	FCombinedChildren AllChildren;

	/** 인디케이터 위젯을 재사용하기 위한 위젯 풀입니다. */
	FUserWidgetPool IndicatorPool;

	/** 화면 밖 인디케이터의 방향을 표시하는 화살표 브러시입니다. */
	const FSlateBrush* ActorCanvasArrowBrush = nullptr;

	/** 다음으로 사용할 방향 화살표 슬롯의 인덱스입니다. */
	mutable int32 NextArrowIndex = 0;

	/** 마지막 갱신에서 사용된 방향 화살표 슬롯의 인덱스입니다. */
	mutable int32 ArrowIndexLastUpdate = 0;

	/** 추가된 순서대로 그려 배칭을 비활성화할지 나타냅니다.
	 * Whether to draw elements in the order they were added to canvas. Note: Enabling this will disable batching and will cause a greater number of drawcalls
	 */
	bool bDrawElementsInOrder = false;

	/** 현재 하나 이상의 인디케이터를 표시하고 있는지 나타냅니다. */
	bool bShowAnyIndicators = false;

	/** 페인트 시점에 사용된 캔버스 지오메트리를 저장합니다. */
	mutable TOptional<FGeometry> OptionalPaintGeometry;

	/** 캔버스를 주기적으로 갱신하는 활성 타이머 핸들입니다. */
	TSharedPtr<FActiveTimerHandle> TickHandle;
};
