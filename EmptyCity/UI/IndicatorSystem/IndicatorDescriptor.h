// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/SceneComponent.h"
#include "Types/SlateEnums.h"

#include "IndicatorDescriptor.generated.h"

class SWidget;
class UIndicatorDescriptor;
class UECIndicatorManagerComponent;
class UUserWidget;
struct FFrame;
struct FSceneViewProjectionData;

/**
 * 인디케이터가 추적하는 3D 월드 위치를 화면상의 2D 좌표로 투영하는 구조체입니다.
 */
struct FIndicatorProjection
{
	/**
	 * Descriptor에 설정된 컴포넌트와 투영 방식을 기준으로 화면 위치를 계산합니다.
	 *
	 * @param IndicatorDescriptor 투영에 필요한 대상 컴포넌트와 배치 설정
	 * @param InProjectionData    현재 로컬 플레이어 카메라의 투영 정보
	 * @param ScreenSize          투영 대상 화면의 크기
	 * @param ScreenPositionWithDepth 계산된 화면 X/Y 좌표와 카메라 거리 Z
	 * @return 대상 위치를 정상적으로 계산했으면 true
	 */
	bool Project(
		const UIndicatorDescriptor& IndicatorDescriptor,
		const FSceneViewProjectionData& InProjectionData,
		const FVector2f& ScreenSize,
		FVector& ScreenPositionWithDepth
		);
};

/**
 * 인디케이터의 월드 위치를 화면 좌표로 변환할 때 사용할 기준 방식을 정의합니다.
 */
UENUM(BlueprintType)
enum class EActorCanvasProjectionMode : uint8
{
	/** 지정된 컴포넌트 또는 소켓의 월드 위치 한 점을 화면에 투영합니다. */
	ComponentPoint,

	/** 지정된 컴포넌트의 3D Bounds 내부에서 기준점을 선택해 투영합니다. */
	ComponentBoundingBox,

	/** 컴포넌트의 3D Bounds 전체를 화면 사각형으로 투영한 뒤 기준점을 선택합니다. */
	ComponentScreenBoundingBox,

	/** 액터 전체의 3D Bounds 내부에서 기준점을 선택해 투영합니다. */
	ActorBoundingBox,

	/** 액터 전체 Bounds를 화면 사각형으로 투영한 뒤 기준점을 선택합니다. */
	ActorScreenBoundingBox
};

/**
 * 화면에 표시될 하나의 인디케이터에 필요한 대상, 위젯, 위치 및 정렬 정보를 보관합니다.
 *
 * 실제 위젯의 생성과 화면 배치는 SActorCanvas가 담당하며,
 * Indicator Descriptor는 그 과정에 필요한 데이터를 제공합니다.
 */
UCLASS(BlueprintType)
class EMPTYCITY_API UIndicatorDescriptor : public UObject
{
	GENERATED_BODY()
	
public:
	UIndicatorDescriptor() { }

public:
	UFUNCTION(BlueprintCallable)
	UObject* GetDataObject() const { return DataObject; }
	UFUNCTION(BlueprintCallable)
	void SetDataObject(UObject* InDataObject) { DataObject = InDataObject; }
	
	UFUNCTION(BlueprintCallable)
	USceneComponent* GetSceneComponent() const { return Component; }
	UFUNCTION(BlueprintCallable)
	void SetSceneComponent(USceneComponent* InComponent) { Component = InComponent; }

	/** 위치 기준으로 사용할 컴포넌트 소켓 이름을 반환합니다. */
	UFUNCTION(BlueprintCallable)
	FName GetComponentSocketName() const { return ComponentSocketName; }
	/** 위치 기준으로 사용할 컴포넌트 소켓 이름을 설정합니다. NAME_None이면 컴포넌트 자체 위치를 사용합니다. */
	UFUNCTION(BlueprintCallable)
	void SetComponentSocketName(FName SocketName) { ComponentSocketName = SocketName; }

	/** 화면에 생성할 인디케이터 UUserWidget 클래스를 반환합니다. */
	UFUNCTION(BlueprintCallable)
	TSoftClassPtr<UUserWidget> GetIndicatorClass() const { return IndicatorWidgetClass; }
	/** 화면에 생성할 인디케이터 UUserWidget 클래스를 설정합니다. */
	UFUNCTION(BlueprintCallable)
	void SetIndicatorClass(TSoftClassPtr<UUserWidget> InIndicatorWidgetClass)
	{
		IndicatorWidgetClass = InIndicatorWidgetClass;
	}

	/** 인디케이터에 표시할 기본 텍스트를 반환합니다. */
	UFUNCTION(BlueprintCallable)
	FText GetMainText() const {	return MainText;	}
	/** 인디케이터에 표시할 기본 텍스트를 설정합니다. */
	UFUNCTION(BlueprintCallable)
	void SetMainText(const FText& InMainText) {	MainText = InMainText; }

	/** 인디케이터에 표시할 보조 텍스트를 반환합니다. */
	UFUNCTION(BlueprintCallable)
	FText GetSubText() const { return SubText; }
	/** 인디케이터에 표시할 보조 텍스트를 설정합니다. */
	UFUNCTION(BlueprintCallable)
	void SetSubText(const FText& InSubText) { SubText = InSubText; }

public:
	// TODO Organize this better.
	/** 현재 이 Descriptor에 연결되어 화면에 표시 중인 UUserWidget입니다. */
	TWeakObjectPtr<UUserWidget> IndicatorWidget;

public:
	/** 추적 대상 Scene Component가 유효하지 않을 때 인디케이터를 자동 제거할지 설정합니다. */
	UFUNCTION(BlueprintCallable)
	void SetAutoRemoveWhenIndicatorComponentIsNull(bool CanAutomaticallyRemove)
	{
		bAutoRemoveWhenIndicatorComponentIsNull = CanAutomaticallyRemove;
	}
	/** 대상 컴포넌트가 사라졌을 때 자동 제거하도록 설정되어 있는지 반환합니다. */
	UFUNCTION(BlueprintCallable)
	bool GetAutoRemoveWhenIndicatorComponentIsNull() const { return bAutoRemoveWhenIndicatorComponentIsNull; }

	/** 자동 제거 옵션이 활성화되어 있고 추적 대상 컴포넌트가 유효하지 않은지 검사합니다. */
	bool CanAutomaticallyRemove() const
	{
		return bAutoRemoveWhenIndicatorComponentIsNull && !IsValid(GetSceneComponent());
	}

public:
// ============================================================================
// Layout Properties
// ============================================================================

	UFUNCTION(BlueprintCallable)
	bool GetIsVisible() const { return IsValid(GetSceneComponent()) && bVisible; }
	
	UFUNCTION(BlueprintCallable)
	void SetDesiredVisibility(bool InVisible)
	{
		bVisible = InVisible;
	}

	/** 월드 위치를 화면 좌표로 변환할 때 사용할 투영 방식을 반환합니다. */
	UFUNCTION(BlueprintCallable)
	EActorCanvasProjectionMode GetProjectionMode() const { return ProjectionMode; }
	/** 월드 위치를 화면 좌표로 변환할 때 사용할 투영 방식을 설정합니다. */
	UFUNCTION(BlueprintCallable)
	void SetProjectionMode(EActorCanvasProjectionMode InProjectionMode)
	{
		ProjectionMode = InProjectionMode;
	}

	/** 투영된 기준점에 대해 위젯을 수평 방향으로 어떻게 정렬할지 반환합니다. */
	UFUNCTION(BlueprintCallable)
	EHorizontalAlignment GetHAlign() const { return HAlignment; }
	/** 투영된 기준점에 대해 위젯을 수평 방향으로 어떻게 정렬할지 설정합니다. */
	UFUNCTION(BlueprintCallable)
	void SetHAlign(EHorizontalAlignment InHAlignment)
	{
		HAlignment = InHAlignment;
	}

	/** 투영된 기준점에 대해 위젯을 수직 방향으로 어떻게 정렬할지 반환합니다. */
	UFUNCTION(BlueprintCallable)
	EVerticalAlignment GetVAlign() const { return VAlignment; }
	/** 투영된 기준점에 대해 위젯을 수직 방향으로 어떻게 정렬할지 설정합니다. */
	UFUNCTION(BlueprintCallable)
	void SetVAlign(EVerticalAlignment InVAlignment)
	{
		VAlignment = InVAlignment;
	}


	/** 인디케이터가 화면 밖으로 나갈 때 화면 가장자리에 고정할지 반환합니다. */
	UFUNCTION(BlueprintCallable)
	bool GetClampToScreen() const { return bClampToScreen; }
	/** 인디케이터가 화면 밖으로 나갈 때 화면 가장자리에 고정할지 설정합니다. */
	UFUNCTION(BlueprintCallable)
	void SetClampToScreen(bool bValue)
	{
		bClampToScreen = bValue;
	}

	/** 화면 가장자리에 고정된 인디케이터에 방향 화살표를 표시할지 반환합니다. */
	UFUNCTION(BlueprintCallable)
	bool GetShowClampToScreenArrow() const { return bShowClampToScreenArrow; }
	/** 화면 가장자리에 고정된 인디케이터에 방향 화살표를 표시할지 설정합니다. */
	UFUNCTION(BlueprintCallable)
	void SetShowClampToScreenArrow(bool bValue)
	{
		bShowClampToScreenArrow = bValue;
	}

	/** 투영 전에 대상의 월드 위치에 더할 3D 오프셋을 반환합니다. */
	UFUNCTION(BlueprintCallable)
	FVector GetWorldPositionOffset() const
	{
		return WorldPositionOffset;
	}

	/** 투영 전에 대상의 월드 위치에 더할 3D 오프셋을 설정합니다. */
	UFUNCTION(BlueprintCallable)
	void SetWorldPositionOffset(FVector Offset)
	{
		WorldPositionOffset = Offset;
	}

	/** 투영이 끝난 화면 좌표에 더할 2D 오프셋을 반환합니다. */
	UFUNCTION(BlueprintCallable)
	FVector2D GetScreenSpaceOffset() const
	{
		return ScreenSpaceOffset;
	}

	/** 투영이 끝난 화면 좌표에 더할 2D 오프셋을 설정합니다. */
	UFUNCTION(BlueprintCallable)
	void SetScreenSpaceOffset(FVector2D Offset)
	{
		ScreenSpaceOffset = Offset;
	}

	/**
	 * 대상 Bounds 내부에서 인디케이터 위치로 사용할 정규화 기준점을 반환합니다.
	 *
	 * 각 축은 일반적으로 0~1 범위를 사용하며,
	 * FVector(0.5, 0.5, 0.5)는 Bounds 중앙을 의미합니다.
	 */
	UFUNCTION(BlueprintCallable)
	FVector GetBoundingBoxAnchor() const
	{
		return BoundingBoxAnchor;
	}

	/** 대상 Bounds 내부에서 인디케이터 위치로 사용할 정규화 기준점을 설정합니다. */
	UFUNCTION(BlueprintCallable)
	void SetBoundingBoxAnchor(FVector InBoundingBoxAnchor)
	{
		BoundingBoxAnchor = InBoundingBoxAnchor;
	}

public:
// ============================================================================
// Sorting Properties
// ============================================================================

	/** 깊이 정렬 이후 인디케이터 간의 추가적인 표시 우선순위를 반환합니다. */
	UFUNCTION(BlueprintCallable)
	int32 GetPriority() const
	{
		return Priority;
	}

	/** 깊이 정렬 이후 인디케이터 간의 추가적인 표시 우선순위를 설정합니다. */
	UFUNCTION(BlueprintCallable)
	void SetPriority(int32 InPriority)
	{
		Priority = InPriority;
	}

public:
	/** 이 Descriptor가 등록된 Indicator Manager Component를 반환합니다. */
	UECIndicatorManagerComponent* GetIndicatorManagerComponent()
	{
		return ManagerPtr.Get();
	}

	/** 이 Descriptor를 관리할 Indicator Manager Component를 설정합니다. */
	void SetIndicatorManagerComponent(UECIndicatorManagerComponent* InManager);

	/** 현재 Manager Component에서 이 인디케이터를 제거합니다. */
	UFUNCTION(BlueprintCallable)
	void UnregisterIndicator();

private:
	UPROPERTY()
	bool bVisible = true;

	/** 인디케이터가 화면 밖으로 나갔을 때 화면 가장자리 안쪽에 고정할지 결정합니다. */
	UPROPERTY()
	bool bClampToScreen = false;

	/** 화면 가장자리에 고정된 인디케이터에 방향 화살표를 표시할지 결정합니다. */
	UPROPERTY()
	bool bShowClampToScreenArrow = false;

	/** 계산된 투영 위치 대신 외부에서 지정한 화면 위치를 사용할지 나타내는 확장용 플래그입니다. */
	UPROPERTY()
	bool bOverrideScreenPosition = false;

	/** 추적 대상 Scene Component가 유효하지 않을 때 Manager에서 인디케이터를 자동 제거할지 결정합니다. */
	UPROPERTY()
	bool bAutoRemoveWhenIndicatorComponentIsNull = false;

	/** 대상의 월드 위치를 화면 좌표로 변환할 때 사용할 투영 방식입니다. */
	UPROPERTY()
	EActorCanvasProjectionMode ProjectionMode =	EActorCanvasProjectionMode::ComponentPoint;

	/** 투영된 기준점을 기준으로 위젯을 배치할 수평 정렬 방식입니다. */
	UPROPERTY()
	TEnumAsByte<EHorizontalAlignment> HAlignment = HAlign_Center;

	/** 투영된 기준점을 기준으로 위젯을 배치할 수직 정렬 방식입니다. */
	UPROPERTY()
	TEnumAsByte<EVerticalAlignment> VAlignment = VAlign_Center;

	UPROPERTY()
	int32 Priority = 0;
	UPROPERTY()
	FVector BoundingBoxAnchor = FVector(0.5, 0.5, 0.5);
	UPROPERTY()
	FVector2D ScreenSpaceOffset = FVector2D(0, 0);
	UPROPERTY()
	FVector WorldPositionOffset = FVector(0, 0, 0);

private:
	friend class SActorCanvas;

	UPROPERTY()
	TObjectPtr<UObject> DataObject;
	
	UPROPERTY()
	TObjectPtr<USceneComponent> Component;
	
	UPROPERTY()
	FName ComponentSocketName = NAME_None;
	
	UPROPERTY()
	TSoftClassPtr<UUserWidget> IndicatorWidgetClass;
	
	UPROPERTY()
	TWeakObjectPtr<UECIndicatorManagerComponent> ManagerPtr;
	
	TWeakPtr<SWidget> Content;
	
	TWeakPtr<SWidget> CanvasHost;

	/** 인디케이터에 표시할 기본 텍스트입니다. */
	UPROPERTY()
	FText MainText;

	/** 인디케이터에 표시할 추가 설명 또는 보조 텍스트입니다. */
	UPROPERTY()
	FText SubText;
};
