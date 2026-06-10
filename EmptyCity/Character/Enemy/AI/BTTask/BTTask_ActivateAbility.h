// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_ActivateAbility.generated.h"

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UBTTask_ActivateAbility : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTTask_ActivateAbility();

protected:	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 실행시킬 Ability Tag입니다. */
	UPROPERTY(EditAnywhere, Category = "Ability Tag")
	FGameplayTag AbilityTag;
};
