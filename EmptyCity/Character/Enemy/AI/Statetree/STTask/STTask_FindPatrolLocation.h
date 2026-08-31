// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STTask_FindPatrolLocation.generated.h"

USTRUCT()
struct FSTTask_FindPatrolLocationInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	FVector InitialLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float SearchRadius = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MinPatrolDistance = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	int32 MaxTries = 3;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 100.0f;
};

/**
 * 
 */
USTRUCT(meta = (DisplayName = "Find Patrol Location"))
struct EMPTYCITY_API FSTTask_FindPatrolLocation : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTTask_FindPatrolLocationInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
