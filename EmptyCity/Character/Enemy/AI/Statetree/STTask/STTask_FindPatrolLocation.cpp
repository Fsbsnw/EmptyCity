// Fill out your copyright notice in the Description page of Project Settings.


#include "STTask_FindPatrolLocation.h"

#include "NavigationSystem.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"
#include "Character/Enemy/AI/ECEnemyAIController.h"

EStateTreeRunStatus FSTTask_FindPatrolLocation::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AECEnemyAIController* AIController = Cast<AECEnemyAIController>(Context.GetOwner());

	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;

	if (!Pawn)
	{
		return EStateTreeRunStatus::Failed;
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn->GetWorld());

	if (!NavSys)
	{
		// NavMesh가 없는 배치도 AI 전체를 실패시키지 않고 PatrolWait에서 재시도합니다.
		return EStateTreeRunStatus::Succeeded;
	}

	const FVector CurrentLocation = Pawn->GetActorLocation();
	const FVector ProjectionExtent(
		FMath::Max(InstanceData.NavProjectionExtent.X, 1.0f),
		FMath::Max(InstanceData.NavProjectionExtent.Y, 1.0f),
		FMath::Max(InstanceData.NavProjectionExtent.Z, 1.0f));
	FNavLocation ProjectedCurrentLocation;
	if (!NavSys->ProjectPointToNavigation(
		CurrentLocation,
		ProjectedCurrentLocation,
		ProjectionExtent))
	{
		// 현재 위치 주변에 NavMesh가 없으면 방향만 반복해서 트는 대신 잠시 대기합니다.
		return EStateTreeRunStatus::Succeeded;
	}

	FNavLocation ProjectedInitialLocation;
	const bool bHasProjectedInitialLocation = NavSys->ProjectPointToNavigation(
		InstanceData.InitialLocation,
		ProjectedInitialLocation,
		ProjectionExtent);
	const FVector PatrolCenter = bHasProjectedInitialLocation
		? ProjectedInitialLocation.Location
		: ProjectedCurrentLocation.Location;
	const float SearchRadius = FMath::Max(InstanceData.SearchRadius, 0.0f);
	const float MinimumPatrolDistance = FMath::Max(InstanceData.MinPatrolDistance, 0.0f);
	const int32 LocationAttemptCount = FMath::Max(InstanceData.MaxTries, 8);
	FNavLocation RandomLocation;

	for (int32 TryIndex = 0; TryIndex < LocationAttemptCount; ++TryIndex)
	{
		// 현재 NavMesh 섬을 기준으로 뽑아, 초기 위치 반대편의 연결되지 않은 NavMesh를 선택하지 않습니다.
		const bool bFoundLocation = NavSys->GetRandomReachablePointInRadius(
			ProjectedCurrentLocation.Location,
			SearchRadius,
			RandomLocation);

		if (!bFoundLocation)
		{
			continue;
		}

		// 현재 위치 기준으로 검색하더라도 최초 배치 지점의 순찰 반경 밖으로 계속 퍼지지 않게 합니다.
		if (FVector::DistSquared2D(PatrolCenter, RandomLocation.Location)
			> FMath::Square(SearchRadius))
		{
			continue;
		}

		const float Distance = FVector::Dist2D(CurrentLocation, RandomLocation.Location);
		
		if (Distance < MinimumPatrolDistance)
		{
			continue;
		}

		const EPathFollowingRequestResult::Type MoveResult =
			AIController->MoveToLocation(
			RandomLocation.Location,
			InstanceData.AcceptanceRadius,
			true,    // bStopOnOverlap
			true,    // bUsePathfinding
			true,    // bProjectDestinationToNavigation
			true,    // bCanStrafe
		nullptr,
		false);  // bAllowPartialPath

		switch (MoveResult)
		{
			case EPathFollowingRequestResult::RequestSuccessful:
				InstanceData.LastProgressLocation = CurrentLocation;
				InstanceData.TimeWithoutProgress = 0.0f;
				InstanceData.MoveElapsedTime = 0.0f;
				return EStateTreeRunStatus::Running;

			case EPathFollowingRequestResult::AlreadyAtGoal:
				return EStateTreeRunStatus::Succeeded;

			default:
				// 한 후보의 경로 요청 실패가 AI 전체 실패가 되지 않도록 다음 후보를 검사합니다.
				continue;
		}
	}

	// 순찰 가능한 곳이 없는 것은 정상적인 환경 조건입니다. 대기 후 다시 시도합니다.
	return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FSTTask_FindPatrolLocation::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	AECEnemyAIController* AIController = Cast<AECEnemyAIController>(Context.GetOwner());
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	UPathFollowingComponent* PathFollowing = AIController
		? AIController->GetPathFollowingComponent()
		: nullptr;

	if (!Pawn || !PathFollowing)
	{
		return EStateTreeRunStatus::Failed;
	}

	const float SafeDeltaTime = FMath::Max(DeltaTime, 0.0f);
	InstanceData.MoveElapsedTime += SafeDeltaTime;

	switch (AIController->GetMoveStatus())
	{
		case EPathFollowingStatus::Moving:
		case EPathFollowingStatus::Waiting:
		{
			const FVector CurrentLocation = Pawn->GetActorLocation();
			if (FVector::DistSquared2D(CurrentLocation, InstanceData.LastProgressLocation)
				>= FMath::Square(FMath::Max(InstanceData.MinProgressDistance, 1.0f)))
			{
				InstanceData.LastProgressLocation = CurrentLocation;
				InstanceData.TimeWithoutProgress = 0.0f;
			}
			else
			{
				InstanceData.TimeWithoutProgress += SafeDeltaTime;
			}

			if (InstanceData.TimeWithoutProgress >= FMath::Max(InstanceData.StuckTimeout, 0.1f)
				|| InstanceData.MoveElapsedTime >= FMath::Max(InstanceData.MaxMoveDuration, 0.1f))
			{
				AIController->StopMovement();
				return EStateTreeRunStatus::Succeeded;
			}

			return EStateTreeRunStatus::Running;
		}

		case EPathFollowingStatus::Paused:
			// StateTree 자체가 일시정지된 경우 Tick도 멈춥니다. 실행 중인데 경로만 Paused라면 제한 시간 후 회수합니다.
			if (InstanceData.MoveElapsedTime >= FMath::Max(InstanceData.StuckTimeout, 0.1f))
			{
				AIController->StopMovement();
				return EStateTreeRunStatus::Succeeded;
			}
			return EStateTreeRunStatus::Running;

		case EPathFollowingStatus::Idle:
			// 경로 중단 역시 PatrolWait 뒤 새 위치를 고르게 하며 StateTree를 실패시키지 않습니다.
			return EStateTreeRunStatus::Succeeded;

		default:
			return EStateTreeRunStatus::Succeeded;
	}
}

void FSTTask_FindPatrolLocation::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	AECEnemyAIController* AIController = Cast<AECEnemyAIController>(Context.GetOwner());

	if (AIController &&	AIController->GetMoveStatus() != EPathFollowingStatus::Idle)
	{
		// 피격 등 다른 Transition으로 Task가 중간 종료된 경우 이동 취소
		AIController->StopMovement();
	}
}
