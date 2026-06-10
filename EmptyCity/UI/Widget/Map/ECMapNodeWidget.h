// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECMapNodeWidget.generated.h"

// 맵 노드가 클릭되었을 때 발생시킬 델리게이트 선언
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMapNodeClicked, class UECMapNodeWidget*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMapNodeHovered, UECMapNodeWidget*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMapNodeUnhovered, UECMapNodeWidget*);

enum class EWidgetNavDirection : uint8;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECMapNodeWidget : public UECUserWidget
{
	GENERATED_BODY()
protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

public:
	/** 내 노드의 고유 식별 태그 (예: MapNode.Location.Home) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node|Info")
	FGameplayTag MyNodeTag;

	/** 각 방향(상/하/좌/우)에 연결된 이웃 노드의 태그들을 저장하는 맵 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node|Info")
	TMap<EWidgetNavDirection, FGameplayTag> NeighborNodes;
	
	/** 클릭 이벤트를 구독할 수 있는 델리게이트 */
	FOnMapNodeClicked OnNodeClicked;
	FOnMapNodeHovered OnNodeHovered;
	FOnMapNodeUnhovered OnNodeUnhovered;

// ========================================================
// 블루프린트 호출 함수(Visual)
// ========================================================
public:
	/** 선택 연출 (WBP에서 애니메이션 재생 등 구현) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Node|Visual")
	void PlayHighlightAnim();
	
	/** 선택 해제 연출 (WBP에서 애니메이션 재생 등 구현) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Node|Visual")
	void PlayUnhighlightAnim();
};
