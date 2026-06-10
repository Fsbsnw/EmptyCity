// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/AI/BTTask/BTTask_ActivateAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"

UBTTask_ActivateAbility::UBTTask_ActivateAbility()
{
	NodeName = TEXT("Activate Ability");
}

EBTNodeResult::Type UBTTask_ActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();

	// 자신의 AbilitySystemComponent를 찾아서 AbilityTag에 해당하는 GA를 실행시킵니다.
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AIController->GetPawn());
	if (ASC)
	{
		ASC->TryActivateAbilitiesByTag(AbilityTag.GetSingleTagContainer());
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}