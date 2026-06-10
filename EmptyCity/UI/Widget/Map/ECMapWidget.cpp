// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Map/ECMapWidget.h"

#include "ECGameplayTags.h"
#include "ECMapNodeWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"
#include "UI/Widget/ECMapNodeInfoWidget.h"

void UECMapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AllMapNodes.Empty();

	// 1. WBP 내부에 배치된 모든 하위 위젯을 순회하며 맵 노드만 찾아 캐싱합니다.
	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([this](UWidget* Widget)
		{
			// 위젯이 맵 노드 클래스인지 확인
			if (UECMapNodeWidget* MapNode = Cast<UECMapNodeWidget>(Widget))
			{
				// 태그가 유효하다면 TMap에 등록
				if (MapNode->MyNodeTag.IsValid())
				{
					AllMapNodes.Add(MapNode->MyNodeTag, MapNode);
					
					// Clicked 이벤트 바인딩
					MapNode->OnNodeClicked.AddUObject(this, &UECMapWidget::HandleNodeClicked);

					// Hovered 이벤트 바인딩 
					MapNode->OnNodeHovered.AddUObject(this, &UECMapWidget::HandleNodeHovered);
					MapNode->OnNodeUnhovered.AddUObject(this, &UECMapWidget::HandleNodeUnhovered);
				}
			}
		});
	}
	
	// 2. 초기 포커스 노드 설정
	if (AllMapNodes.Contains(DefaultNodeTag))
	{
		CurrentFocusedNode = AllMapNodes[DefaultNodeTag];
	}

	// 3. 시작 노드에 시각적 하이라이트 켜기
	if (CurrentFocusedNode)
	{
		CurrentFocusedNode->PlayHighlightAnim();
	}
}

void UECMapWidget::ReceiveConfirmInput_Implementation()
{
	if (CurrentFocusedNode == nullptr)
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Focused Node : %s"), *CurrentFocusedNode->MyNodeTag.ToString());
}

void UECMapWidget::ReceiveDirectionInput_Implementation(EWidgetNavDirection Direction)
{
	// 선택되지 않은 상태면 Home을 표시합니다.
	if (CurrentFocusedNode == nullptr)
	{
		CurrentFocusedNode = AllMapNodes[ECGameplayTags::MapNode_Location_Home];
		CurrentFocusedNode->PlayHighlightAnim();
		UpdateNodeInfo(CurrentFocusedNode);
		return;
	}
	// 1. 현재 노드에서 입력된 방향(Direction)에 연결된 이웃 노드 태그를 가져옵니다.
	if (const FGameplayTag* TargetTag = CurrentFocusedNode->NeighborNodes.Find(Direction))
	{
		// 2. 그 방향에 이웃 태그가 등록되어 있고, 유효하다면
		if (TargetTag->IsValid() && AllMapNodes.Contains(*TargetTag))
		{
			// 3. 새로 이동할 노드를 찾습니다.
			UECMapNodeWidget* NextNode = AllMapNodes[*TargetTag];

			// 4. 기존 노드의 하이라이트를 끕니다.
			CurrentFocusedNode->PlayUnhighlightAnim();

			// 5. 새 노드에 하이라이트를 켭니다.
			NextNode->PlayHighlightAnim();

			// 6. 현재 포커싱된 노드를 새 노드로 교체합니다.
			CurrentFocusedNode = NextNode;

			// 변경된 노드 정보를 띄웁니다.
			UpdateNodeInfo(CurrentFocusedNode);
		}
	}
}

void UECMapWidget::HandleNodeClicked(UECMapNodeWidget* ClickedNode)
{
	// 유효성 검사 및 이미 선택된 노드를 또 클릭했는지 방어
	if (ClickedNode == nullptr || CurrentFocusedNode == ClickedNode)
	{
		return;
	}

	// 1. 기존 노드 하이라이트 끄기
	if (CurrentFocusedNode)
	{
		CurrentFocusedNode->PlayUnhighlightAnim();
	}

	// 2. 포커스 교체
	CurrentFocusedNode = ClickedNode;

	// 3. 새 노드 하이라이트 켜기
	CurrentFocusedNode->PlayHighlightAnim();

	// 변경된 노드 정보를 띄웁니다.
	UpdateNodeInfo(CurrentFocusedNode);
}

void UECMapWidget::HandleNodeHovered(UECMapNodeWidget* HoveredNode)
{
	UpdateNodeInfo(HoveredNode);
}

void UECMapWidget::HandleNodeUnhovered(UECMapNodeWidget* UnhoveredNode)
{
	UpdateNodeInfo(CurrentFocusedNode);
}

void UECMapWidget::UpdateNodeInfo(UECMapNodeWidget* TargetNode)
{
	if (!NodeInfoWidget) return;

	if (TargetNode)
	{
		// TODO: TargetNode->MyNodeTag 값을 읽어서 데이터 테이블에서 텍스트, 아이콘 등을 가져와 NodeInfoWidget에 세팅
		FString TagString = TargetNode->MyNodeTag.ToString();
		NodeInfoWidget->SetMapNodeName(FText::FromString(TagString));
		
		// 위치 동기화 로직 : 정보창과 타겟 노드의 캔버스 슬롯을 가져옵니다.
		UCanvasPanelSlot* InfoSlot = Cast<UCanvasPanelSlot>(NodeInfoWidget->Slot);
		if (InfoSlot)
		{
			// 1. 타겟 노드가 현재 모니터 화면상 어디에 있는지 '절대 좌표'를 가져옵니다.
			FVector2D TargetAbsolutePos = TargetNode->GetCachedGeometry().GetAbsolutePosition();

			// 2. 그 절대 좌표를 맵 위젯(this) 캔버스 기준의 '로컬 좌표'로 변환합니다.
			FVector2D LocalPos = GetCachedGeometry().AbsoluteToLocal(TargetAbsolutePos);

			// 3. 변환된 좌표에 오프셋을 더해서 정보창 위치를 설정합니다.
			InfoSlot->SetPosition(LocalPos + NodeInfoWidgetOffset);
		}
		
		NodeInfoWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		// 타겟이 없으면 (예: 맵 처음 열었을 때 포커스가 꼬였을 경우) 숨김
		NodeInfoWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}