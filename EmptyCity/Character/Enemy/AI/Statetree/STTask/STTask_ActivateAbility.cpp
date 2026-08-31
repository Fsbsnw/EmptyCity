// Fill out your copyright notice in the Description page of Project Settings.


#include "STTask_ActivateAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"

FSTTask_ActivateAbility::FSTTask_ActivateAbility()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FSTTask_ActivateAbility::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	AAIController* AIController = Cast<AAIController>(Context.GetOwner());

	if (!AIController || !AIController->GetPawn())
	{
		return EStateTreeRunStatus::Failed;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AIController->GetPawn());

	if (!ASC || !InstanceData.AbilityTag.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	const bool bActivated = ASC->TryActivateAbilitiesByTag(InstanceData.AbilityTag.GetSingleTagContainer());

	return bActivated ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FSTTask_ActivateAbility::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AAIController* AIController = Cast<AAIController>(Context.GetOwner());

	if (!AIController || !AIController->GetPawn())
	{
		return EStateTreeRunStatus::Failed;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AIController->GetPawn());

	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	TArray<FGameplayAbilitySpec*> MatchingAbilities;

	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(
		InstanceData.AbilityTag.GetSingleTagContainer(),
		MatchingAbilities,
		false
	);

	for (const FGameplayAbilitySpec* AbilitySpec : MatchingAbilities)
	{
		if (AbilitySpec && AbilitySpec->IsActive())
		{
			return EStateTreeRunStatus::Running;
		}
	}

	return EStateTreeRunStatus::Succeeded;
}