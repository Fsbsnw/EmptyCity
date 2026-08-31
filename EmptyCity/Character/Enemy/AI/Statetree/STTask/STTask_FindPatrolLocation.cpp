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
		return EStateTreeRunStatus::Failed;
	}

	const FVector CurrentLocation = Pawn->GetActorLocation();
	FNavLocation RandomLocation;

	UE_LOG(LogTemp, Warning, TEXT("Initial : %s"), *InstanceData.InitialLocation.ToString());
	
	for (int32 TryIndex = 0; TryIndex < InstanceData.MaxTries; ++TryIndex)
	{
		const bool bFoundLocation = NavSys->GetRandomReachablePointInRadius(InstanceData.InitialLocation, InstanceData.SearchRadius,RandomLocation);

		if (!bFoundLocation)
		{
			continue;
		}

		const float Distance = FVector::Distance(CurrentLocation, RandomLocation.Location);
		
		if (Distance < InstanceData.MinPatrolDistance)
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
				return EStateTreeRunStatus::Running;

			case EPathFollowingRequestResult::AlreadyAtGoal:
				return EStateTreeRunStatus::Succeeded;

			default:
				return EStateTreeRunStatus::Failed;
		}
	}

	return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FSTTask_FindPatrolLocation::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	AECEnemyAIController* AIController = Cast<AECEnemyAIController>(Context.GetOwner());
	UPathFollowingComponent* PathFollowing = AIController->GetPathFollowingComponent();

	if (!PathFollowing)
	{
		return EStateTreeRunStatus::Failed;
	}

	switch (AIController->GetMoveStatus())
	{
		case EPathFollowingStatus::Moving:
		case EPathFollowingStatus::Waiting:
		case EPathFollowingStatus::Paused:
			return EStateTreeRunStatus::Running;

		case EPathFollowingStatus::Idle:
			return PathFollowing->DidMoveReachGoal() ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;

		default:
			return EStateTreeRunStatus::Failed;
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