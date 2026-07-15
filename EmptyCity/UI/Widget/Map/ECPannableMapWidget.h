// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECPannableMapWidget.generated.h"

class UECMapNodeWidget;
class UCanvasPanel;
class UImage;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECPannableMapWidget : public UECUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	/**
	 * 맵을 Panning하기 위한 함수들입니다.
	 */
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UFUNCTION(BlueprintCallable)
	void FocusMapAtLocalPosition(const FVector2D& MapLocalPosition, float TargetScale);

	UPROPERTY(EditAnywhere)
	float TargetZoomInScale = 2.f;

private:
	FWidgetTransform GetClampedMapTransform(FWidgetTransform Transform) const;

private:
	bool bIsFocusAnimating = false;

	float FocusAnimElapsed = 0.f;

	UPROPERTY(EditAnywhere, Category="Map|Focus")
	float FocusAnimDuration = 0.35f;

	FWidgetTransform FocusStartTransform;
	FWidgetTransform FocusTargetTransform;

	UPROPERTY()
	TMap<FGameplayTag, UECMapNodeWidget*> AllMapNodes;

	UFUNCTION()
	void HandleMapNodeClicked(UECMapNodeWidget* MapNodeWidget);
	
	void ClampMapTransform();
	
protected:
	// WBP에 배치될 맵 이미지 위젯
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UCanvasPanel* CanvasPanel_Root;
	
	// WBP에 배치될 맵 이미지 위젯
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* Image_Map;
	
private:
	/** Drag 상태 추적용 변수 */
	bool bIsDragging = false;

	/** 드래그가 시작된 마우스의 화면 좌표입니다. */
	FVector2D DragStartMousePosition;
    
	/** 드래그가 시작될 때 위젯의 로컬 렌더 트랜스폼 좌표입니다. */
	FVector2D DragStartWidgetTranslation;
};
