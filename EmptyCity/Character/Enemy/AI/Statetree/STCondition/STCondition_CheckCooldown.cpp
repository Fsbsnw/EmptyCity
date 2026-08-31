#include "STCondition_CheckCooldown.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"

bool FSTCondition_CheckCooldown::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!IsValid(InstanceData.AbilityOwner))
	{
		return false;
	}

	if (!InstanceData.CooldownTag.IsValid())
	{
		return false;
	}

	const UAbilitySystemComponent* AbilitySystemComponent =	UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InstanceData.AbilityOwner);

	if (!IsValid(AbilitySystemComponent))
	{
		return false;
	}

	const bool bIsCoolingDown =	AbilitySystemComponent->HasMatchingGameplayTag(InstanceData.CooldownTag);

	switch (InstanceData.RequiredState)
	{
		case EAbilityCooldownState::Ready:
			return !bIsCoolingDown;

		case EAbilityCooldownState::CoolingDown:
			return bIsCoolingDown;

		default:
			return false;
	}
}