#include "STTask_SetFocusToTarget.h"

#include "AIController.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FSTTask_SetFocusToTarget::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!IsValid(InstanceData.SourceAIController))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.bShouldFocusTarget)
	{
		if (!IsValid(InstanceData.TargetActor))
		{
			return EStateTreeRunStatus::Failed;
		}

		InstanceData.SourceAIController->SetFocus(InstanceData.TargetActor, EAIFocusPriority::Gameplay);
	}
	else
	{
		InstanceData.SourceAIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	return EStateTreeRunStatus::Running;
}