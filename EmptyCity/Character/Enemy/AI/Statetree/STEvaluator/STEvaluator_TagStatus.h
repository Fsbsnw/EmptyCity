#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreePropertyRef.h"
#include "STEvaluator_TagStatus.generated.h"

class AAIController;

USTRUCT()
struct FSTEvaluator_TagStatusInstanceData
{
	GENERATED_BODY()

	/** 이 StateTree를 실행 중인 AIController입니다. */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AAIController> AIController = nullptr;

	/** 태그 보유 여부를 전달할 StateTree Bool 변수입니다. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TStateTreePropertyRef<bool> StatusRef;

	/** 예: Status.Debuff.Stun */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag StatusTag;

	/** 상태가 변경됐을 때 보낼 이벤트. 예: GameplayEvent.Debuff.Stun */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag StatusChangedEventTag;

	/** 이전 태그 보유 상태입니다. */
	UPROPERTY(Transient)
	bool bPreviousState = false;

	/** 최초 검사 여부입니다. */
	UPROPERTY(Transient)
	bool bInitialized = false;
};

USTRUCT(meta = (DisplayName = "Gameplay Tag Status"))
struct EMPTYCITY_API FSTEvaluator_TagStatus : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTEvaluator_TagStatusInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};