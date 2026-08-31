#pragma once

#include "CoreMinimal.h"
#include "StateTreePropertyRef.h"
#include "StateTreeTaskBase.h"
#include "STTask_GetInitialLocation.generated.h"

class APawn;

USTRUCT()
struct FSTTask_GetInitialLocationInstanceData
{
	GENERATED_BODY()

	/** 위치를 가져올 AI Pawn입니다. */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<APawn> ControlledPawn = nullptr;

	/** 초기 위치를 저장할 StateTree FVector 변수입니다. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TStateTreePropertyRef<FVector> InitialLocationRef;
};

USTRUCT(meta = (DisplayName = "Get Initial Location"))
struct EMPTYCITY_API FSTTask_GetInitialLocation : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTTask_GetInitialLocationInstanceData;
	
	FSTTask_GetInitialLocation()
	{
		// EnterState에서 한 번만 처리하므로 Tick은 필요 없습니다.
		bShouldCallTick = false;
	}

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};