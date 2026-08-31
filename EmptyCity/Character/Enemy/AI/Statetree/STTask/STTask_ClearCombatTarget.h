#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STTask_ClearCombatTarget.generated.h"

class AECEnemyAIController;

USTRUCT()
struct FSTTask_ClearCombatTargetInstanceData
{
	GENERATED_BODY()

	/** 전투 대상을 관리하는 Enemy AIController입니다. */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AECEnemyAIController> AIController = nullptr;
};

USTRUCT(meta = (DisplayName = "Clear Combat Target"))
struct EMPTYCITY_API FSTTask_ClearCombatTarget : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTTask_ClearCombatTargetInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,	const FStateTreeTransitionResult& Transition) const override;
};