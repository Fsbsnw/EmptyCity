#include "STTask_GetInitialLocation.h"

#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FSTTask_GetInitialLocation::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!IsValid(InstanceData.ControlledPawn))
	{
		return EStateTreeRunStatus::Failed;
	}

	FVector* InitialLocation = InstanceData.InitialLocationRef.GetMutablePtr(Context);

	if (InitialLocation == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	*InitialLocation = InstanceData.ControlledPawn->GetActorLocation();

	return EStateTreeRunStatus::Running;
}