// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/AI/BTTask/BTTask_FindPatrolLocation.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FindPatrolLocation::UBTTask_FindPatrolLocation()
{
	NodeName = "Find Patrol Location";
}

EBTNodeResult::Type UBTTask_FindPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
    
	if (!BlackboardComp || !AIController || !AIController->GetPawn())
	{
		return EBTNodeResult::Failed;
	}

	// 1. 초기 위치와 현재 위치 가져오기
	FVector InitialLocation = BlackboardComp->GetValueAsVector("InitialLocation");
	FVector CurrentLocation = AIController->GetPawn()->GetActorLocation();
	FNavLocation RandomLocation;

	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		bool bFoundValidLocation = false;
        
		int32 MaxTries = 3; // 최대 3번만 찾아보고 포기 (무한루프 방지)

		// 2. 임계값 이상의 거리를 찾을 때까지 반복 (최대 MaxTries 횟수 제한)
		for (int32 i = 0; i < MaxTries; ++i)
		{
			if (NavSys->GetRandomReachablePointInRadius(InitialLocation, SearchRadius, RandomLocation))
			{
				// 찾은 좌표와 현재 위치의 거리를 계산
				const float DistanceToNewLocation = FVector::Distance(CurrentLocation, RandomLocation.Location);

				if (DistanceToNewLocation >= MinPatrolDistance)
				{
					bFoundValidLocation = true;
					break; // 최소 순찰 거리 조건 만족
				}
			}
		}

		// 3. 유효한 좌표를 찾았다면 블랙보드에 적용
		if (bFoundValidLocation)
		{
			BlackboardComp->SetValueAsVector(BlackboardKey.SelectedKeyName, RandomLocation.Location);
			return EBTNodeResult::Succeeded;
		}
	}

	// 길찾기에 실패했거나(NavMesh 없음), 3번 다 찾았는데도 너무 가까운 곳만 나왔다면 실패 처리
	return EBTNodeResult::Failed;
}
