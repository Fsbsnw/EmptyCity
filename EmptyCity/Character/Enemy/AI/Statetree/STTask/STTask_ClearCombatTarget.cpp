#include "STTask_ClearCombatTarget.h"

#include "StateTreeExecutionContext.h"
#include "Character/Enemy/AI/ECEnemyAIController.h"

EStateTreeRunStatus FSTTask_ClearCombatTarget::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData =	Context.GetInstanceData(*this);

	if (!IsValid(InstanceData.AIController))
	{
		UE_LOG(LogTemp,	Error, TEXT("ClearCombatTarget 실패: AIController가 유효하지 않습니다."));

		return EStateTreeRunStatus::Failed;
	}

	InstanceData.AIController->ClearCombatTarget();

	return EStateTreeRunStatus::Succeeded;
}