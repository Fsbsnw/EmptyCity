// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECMapWidget.generated.h"

// 위젯 탐색 시 사용되는 입력 방향입니다.
UENUM(BlueprintType)
enum class EWidgetNavDirection : uint8
{
	Up, Down, Left, Right
};

class UECMapNodeInfoWidget;
class UECMapNodeWidget;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECMapWidget : public UECUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void OnWidgetOpened() override;

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

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

	/** 새롭게 해금된 노드의 연출 대기열 */
	UPROPERTY()
	TArray<UECMapNodeWidget*> NewRevealNodeQueue;

private:
	/** 맵 노드가 클릭될 때 호출되는 함수입니다. */
	UFUNCTION()
	void HandleNodeClicked(UECMapNodeWidget* ClickedNode);

	/** Hovered 이벤트를 담당하는 함수입니다. */
	void HandleNodeHovered(UECMapNodeWidget* HoveredNode);
	void HandleNodeUnhovered(UECMapNodeWidget* UnhoveredNode);

	/** 해당 노드의 위젯 정보를 보여줍니다. */
	void UpdateNodeInfo(UECMapNodeWidget* TargetNode);

	// 다음 연출을 실행하는 함수
	void ProcessNextReveal();

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Map|Visual")
	void PlayNewNodeHighlight(const FVector2D& TargetTranslation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Map|Visual")
	void PlayNewNodeUnhighlight();
};