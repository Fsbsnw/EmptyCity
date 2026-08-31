#include "STTask_RotateToTarget.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

FSTTask_RotateToTarget::FSTTask_RotateToTarget()
{
	bShouldCallTick = true;
	bShouldStateChangeOnReselect = true;
}

EStateTreeRunStatus FSTTask_RotateToTarget::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.ElapsedTime = 0.0f;

	if (!IsValid(InstanceData.SourceAIController)
		|| !IsValid(InstanceData.SourceAIController->GetPawn())
		|| !IsValid(InstanceData.TargetActor))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.SourceAIController->SetFocus(InstanceData.TargetActor, EAIFocusPriority::Gameplay);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTTask_RotateToTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AAIController* AIController = InstanceData.SourceAIController;
	AActor* TargetActor = InstanceData.TargetActor;
	APawn* Pawn = IsValid(AIController) ? AIController->GetPawn() : nullptr;

	if (!IsValid(Pawn) || !IsValid(TargetActor))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.ElapsedTime += DeltaTime;

	if (InstanceData.Timeout > 0.0f	&& InstanceData.ElapsedTime >= InstanceData.Timeout)
	{
		return EStateTreeRunStatus::Failed;
	}

	FVector DirectionToTarget = TargetActor->GetActorLocation() - Pawn->GetActorLocation();
	DirectionToTarget.Z = 0.0f;

	if (DirectionToTarget.IsNearlyZero())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	const float DesiredYaw = DirectionToTarget.Rotation().Yaw;
	const FRotator CurrentRotation = Pawn->GetActorRotation();
	const float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, DesiredYaw);

	if (FMath::Abs(DeltaYaw) <= InstanceData.AcceptableAngle)
	{
		FRotator FinalRotation = CurrentRotation;
		FinalRotation.Yaw = DesiredYaw;
		Pawn->SetActorRotation(FinalRotation);
		return EStateTreeRunStatus::Succeeded;
	}

	FRotator NewRotation = CurrentRotation;
	NewRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw,	DesiredYaw,InstanceData.RotationSpeed * DeltaTime);

	Pawn->SetActorRotation(NewRotation);

	FRotator ControlRotation = AIController->GetControlRotation();
	ControlRotation.Yaw = NewRotation.Yaw;
	AIController->SetControlRotation(ControlRotation);

	return EStateTreeRunStatus::Running;
}

void FSTTask_RotateToTarget::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.bClearFocusOnExit && IsValid(InstanceData.SourceAIController))
	{
		InstanceData.SourceAIController->ClearFocus(EAIFocusPriority::Gameplay);
	}
}