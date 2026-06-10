// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_SetFocusTarget.generated.h"

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UBTService_SetFocusTarget : public UBTService_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTService_SetFocusTarget();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
