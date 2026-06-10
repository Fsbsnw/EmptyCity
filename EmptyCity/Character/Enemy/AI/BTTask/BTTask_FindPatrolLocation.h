// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FindPatrolLocation.generated.h"

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UBTTask_FindPatrolLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTTask_FindPatrolLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 처음 위치 기준, 탐색 반경입니다. */
	UPROPERTY(EditAnywhere)
	float SearchRadius = 500.f;

	/** 다음 순찰 거리가 현재 위치 기준에서 최소한으로 떨어져야 하는 거리입니다. */
	UPROPERTY(EditAnywhere)
	float MinPatrolDistance = 150.0f;
};
