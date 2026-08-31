#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "STCondition_TargetInRange.generated.h"

USTRUCT()
struct FSTC_TargetInRangeInstanceData
{
	GENERATED_BODY()

	/** 거리 검사의 기준이 되는 액터입니다. */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> SelfActor = nullptr;

	/** 거리 검사 대상입니다. */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	/** 이 거리 이하이면 조건을 통과합니다. */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
	float Range = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bReverse = false;
};

USTRUCT()
struct EMPTYCITY_API FSTCondition_TargetInRange : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTC_TargetInRangeInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};