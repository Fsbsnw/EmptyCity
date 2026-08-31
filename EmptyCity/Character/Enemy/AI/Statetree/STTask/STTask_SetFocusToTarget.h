#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STTask_SetFocusToTarget.generated.h"

class AActor;
class AAIController;

USTRUCT()
struct FSTTask_SetFocusToTargetInstanceData
{
	GENERATED_BODY()

	/** 포커스를 설정하거나 해제할 AIController입니다. */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AAIController> SourceAIController = nullptr;

	/** 포커스를 설정할 대상입니다. */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	/** true이면 TargetActor에 포커스하고, false이면 포커스를 해제합니다. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bShouldFocusTarget = true;
};

USTRUCT(meta = (DisplayName = "Set Focus To Target"))
struct EMPTYCITY_API FSTTask_SetFocusToTarget : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTTask_SetFocusToTargetInstanceData;

	FSTTask_SetFocusToTarget()
	{
		bShouldCallTick = false;
	}

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};