// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Widget/ECUserWidget.h"
#include "UI/Widget/Interface/NavigableWidgetInterface.h"
#include "ECMapWidget.generated.h"

class UECMapNodeInfoWidget;
class UECMapNodeWidget;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECMapWidget : public UECUserWidget, public INavigableWidgetInterface
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

// =========================================================================
// INavigableWidget Interface
// =========================================================================
public:
	virtual void ReceiveConfirmInput_Implementation() override;
	virtual void ReceiveDirectionInput_Implementation(EWidgetNavDirection Direction) override;

// =========================================================================
// 맵 설정 및 상태 관리
// =========================================================================
protected:
	// WBP에 배치될 정보창 위젯
	UPROPERTY(meta = (BindWidget))
	UECMapNodeInfoWidget* NodeInfoWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Map Settings|InfoWidget")
	FVector2D NodeInfoWidgetOffset;
	
	/** 맵을 최초로 열었을 때 선택될 노드 태그 */
	UPROPERTY(EditDefaultsOnly, Category = "Map Settings")
	FGameplayTag DefaultNodeTag;
	
	/** 맵 위젯에 등록된 모든 맵 노드들을 캐싱하는 변수입니다. */
	UPROPERTY()
	TMap<FGameplayTag, UECMapNodeWidget*> AllMapNodes;

	/** 현재 선택된 맵 노드입니다. */
	UPROPERTY()
	UECMapNodeWidget* CurrentFocusedNode;

private:
	/** 맵 노드가 클릭될 때 호출되는 함수입니다. */
	UFUNCTION()
	void HandleNodeClicked(UECMapNodeWidget* ClickedNode);

	/** Hovered 이벤트를 담당하는 함수입니다. */
	void HandleNodeHovered(UECMapNodeWidget* HoveredNode);
	void HandleNodeUnhovered(UECMapNodeWidget* UnhoveredNode);

	/** 해당 노드의 위젯 정보를 보여줍니다. */
	void UpdateNodeInfo(UECMapNodeWidget* TargetNode);
};