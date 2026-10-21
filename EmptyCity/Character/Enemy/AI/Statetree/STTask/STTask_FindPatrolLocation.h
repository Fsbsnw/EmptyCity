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
	int32 MaxTries = 12;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 100.0f;

	/** 현재 위치와 최초 순찰 중심을 NavMesh에 투영할 검색 범위입니다. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FVector NavProjectionExtent = FVector(250.0f, 250.0f, 500.0f);

	/** 이 시간 동안 충분히 이동하지 못하면 현재 순찰 이동을 포기하고 다음 순찰을 준비합니다. */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.1", Units = "s"))
	float StuckTimeout = 2.0f;

	/** 정체 시간 측정을 초기화할 최소 수평 이동 거리입니다. */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0", Units = "cm"))
	float MinProgressDistance = 10.0f;

	/** 하나의 순찰 이동이 무한히 유지되지 않도록 제한하는 최대 시간입니다. */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.1", Units = "s"))
	float MaxMoveDuration = 20.0f;

	UPROPERTY(Transient)
	FVector LastProgressLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	float TimeWithoutProgress = 0.0f;

	UPROPERTY(Transient)
	float MoveElapsedTime = 0.0f;
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
