#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STTask_RotateToTarget.generated.h"

class AActor;
class AAIController;

USTRUCT()
struct EMPTYCITY_API FSTTask_RotateToTargetInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AAIController> SourceAIController = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	/** 초당 회전 속도입니다. */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0", Units = "DegreesPerSecond"))
	float RotationSpeed = 540.0f;

	/** 목표 방향에 도달했다고 판단할 최대 Yaw 오차입니다. */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0", ClampMax = "180.0", Units = "Degrees"))
	float AcceptableAngle = 7.0f;

	/** 제한 시간입니다. 0 이하면 제한 시간을 사용하지 않습니다. */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0", Units = "Seconds"))
	float Timeout = 2.0f;

	/** 상태 종료 시 이 태스크가 설정한 AI Focus를 해제할지 결정합니다. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bClearFocusOnExit = false;

	UPROPERTY(Transient)
	float ElapsedTime = 0.0f;
};

USTRUCT(meta = (DisplayName = "Rotate To Target"))
struct EMPTYCITY_API FSTTask_RotateToTarget : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTTask_RotateToTargetInstanceData;

	FSTTask_RotateToTarget();

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,	const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context,	const FStateTreeTransitionResult& Transition) const override;
};
