// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Map/ECMapWidget.h"

#include "ECGameplayTags.h"
#include "ECMapNodeWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"
#include "Subsystem/ECProgressionSubsystem.h"
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

void UECMapWidget::OnWidgetOpened()
{
	Super::OnWidgetOpened();

	UECProgressionSubsystem* Progression = GetGameInstance()->GetSubsystem<UECProgressionSubsystem>();
	if (!Progression) return;

	NewRevealNodeQueue.Empty(); // 대기열 초기화

	// 1. 모든 노드의 상태를 최신화하고, 연출이 필요한 애들을 찾아냅니다.
	for (const auto& Pair : AllMapNodes)
	{
		bool bIsUnlocked = Progression->IsContentUnlocked(Pair.Value->MyNodeTag);
		bool bIsNewReveal = Progression->CheckAndConsumeReveal(Pair.Value->MyNodeTag);

		// 배우(노드)에게 상태 지시
		Pair.Value->SyncNodeState(bIsUnlocked, bIsNewReveal);

		// 연출이 필요하다면 대기열에 추가!
		if (bIsNewReveal)
		{
			NewRevealNodeQueue.Add(Pair.Value);
		}
	}

	// 2. 연출할 게 하나라도 있다면 순차 연출 시작
	if (NewRevealNodeQueue.Num() > 0)
	{
		ProcessNextReveal();
	}
}

FReply UECMapWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// 1. 현재 입력된 키 가져오기
	const FKey PressedKey = InKeyEvent.GetKey();

	// 2. WASD 키가 아니라면 처리를 안 하고 부모 클래스로 넘김 (무시)
	if (PressedKey != EKeys::W && PressedKey != EKeys::A && PressedKey != EKeys::S && PressedKey != EKeys::D)
	{
		return FReply::Unhandled();
	}

	// 선택되지 않은 상태면 Home을 표시합니다. (WASD 중 아무거나 눌렀을 때 최초 활성화)
	if (CurrentFocusedNode == nullptr)
	{
		CurrentFocusedNode = AllMapNodes[ECGameplayTags::MapNode_Location_Home];
		CurrentFocusedNode->PlayHighlightAnim();
		UpdateNodeInfo(CurrentFocusedNode);
		return FReply::Handled(); // 입력 처리 완료
	}

	// 3. 프로젝트의 Direction 타입에 맞게 키 매핑하기
	// ※ 중요: 'YourDirectionType'을 현재 NeighborNodes의 Key 타입(예: EMoveDirection, EUINavigation 등)으로 바꿔주세요.
	EWidgetNavDirection TargetDirection; 

	if (PressedKey == EKeys::W)       TargetDirection = EWidgetNavDirection::Up;
	else if (PressedKey == EKeys::S)  TargetDirection = EWidgetNavDirection::Down;
	else if (PressedKey == EKeys::A)  TargetDirection = EWidgetNavDirection::Left;
	else if (PressedKey == EKeys::D)  TargetDirection = EWidgetNavDirection::Right;

	// 4. 매핑된 방향(TargetDirection)으로 이웃 노드 검색
	if (const FGameplayTag* TargetTag = CurrentFocusedNode->NeighborNodes.Find(TargetDirection))
	{
		if (TargetTag->IsValid() && AllMapNodes.Contains(*TargetTag))
		{
			// 새 노드로 이동 처리
			UECMapNodeWidget* NextNode = AllMapNodes[*TargetTag];

			CurrentFocusedNode->PlayUnhighlightAnim();
			NextNode->PlayHighlightAnim();
			CurrentFocusedNode = NextNode;

			UpdateNodeInfo(CurrentFocusedNode);
		}
	}

	// WASD 입력을 이 위젯에서 소비(Handled)했으므로, 다른 UI나 슬레이트로 입력이 전파되지 않도록 합니다.
	return FReply::Handled();
}

void UECMapWidget::ProcessNextReveal()
{
	// 대기열이 비었다면 연출 끝!
	if (NewRevealNodeQueue.Num() == 0)
	{
		return;
	}

	// 대기열에서 맨 앞의 노드를 꺼냅니다.
	UECMapNodeWidget* TargetNode = NewRevealNodeQueue[0];
	NewRevealNodeQueue.RemoveAt(0);

	FGeometry ParentGeometry = GetCachedGeometry();
	FGeometry NodeGeometry = TargetNode->GetCachedGeometry();

	FVector2D RealPixelPosition = USlateBlueprintLibrary::AbsoluteToLocal(ParentGeometry, NodeGeometry.GetAbsolutePosition());

	// 2. 전체 맵의 가로/세로 크기 구하기
	FVector2D MapSize = ParentGeometry.GetLocalSize();

	// 3. 픽셀 위치를 0.0 ~ 1.0 사이의 비율(Pivot 좌표)로 변환
	FVector2D TargetPivot = FVector2D::ZeroVector;
	if (MapSize.X > 0.f && MapSize.Y > 0.f)
	{
		TargetPivot.X = RealPixelPosition.X / MapSize.X;
		TargetPivot.Y = RealPixelPosition.Y / MapSize.Y;
	}

	// 4. 이동 대신, '이 피벗을 기준으로 확대해라' 함수 호출
	PlayNewNodeHighlight(TargetPivot);
	
	// // 카메라 이동 연출 실행
	// PlayNewNodeHighlight(RealPixelPosition);

	// 카메라 이동이 끝날 시간(예: 1.5초)을 기다렸다가 다음 노드로 넘어갑니다.
	FTimerHandle WaitHandle;
	GetWorld()->GetTimerManager().SetTimer(WaitHandle, this, &ThisClass::ProcessNextReveal, 1.5f, false);
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
		
		// 1. 타겟 노드가 현재 모니터 화면상 어디에 있는지 '절대 좌표'를 가져옵니다.
		FVector2D TargetAbsolutePos = TargetNode->GetCachedGeometry().GetAbsolutePosition();

		// 2. 그 절대 좌표를 맵 위젯(this) 캔버스 기준의 '로컬 좌표'로 변환합니다.
		FVector2D LocalPos = GetCachedGeometry().AbsoluteToLocal(TargetAbsolutePos);

		// 3. 변환된 좌표에 오프셋을 더해서 정보창 위치를 설정합니다.
		NodeInfoWidget->SetRenderTranslation(LocalPos + NodeInfoWidgetOffset);
		
		NodeInfoWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		// 타겟이 없으면 (예: 맵 처음 열었을 때 포커스가 꼬였을 경우) 숨김
		NodeInfoWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}