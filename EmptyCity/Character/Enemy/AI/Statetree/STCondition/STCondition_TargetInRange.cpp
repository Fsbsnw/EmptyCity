#include "STCondition_TargetInRange.h"

#include "StateTreeExecutionContext.h"

bool FSTCondition_TargetInRange::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!IsValid(InstanceData.SelfActor) || !IsValid(InstanceData.TargetActor))
	{
		return false;
	}

	const bool bIsInRange = InstanceData.SelfActor->GetDistanceTo(InstanceData.TargetActor) <= InstanceData.Range;

	return InstanceData.bReverse ? !bIsInRange : bIsInRange;
}