#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "StateTreeTaskBase.h"
#include "STTask_ApplyGEWhileActive.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

/**
 * StateTree 상태가 활성화된 동안 GameplayEffect를 유지하기 위한 InstanceData입니다.
 */
USTRUCT()
struct EMPTYCITY_API FSTTask_ApplyGEWhileActiveInstanceData
{
    GENERATED_BODY()

    /**
     * 상태가 활성화된 동안 적용할 GameplayEffect입니다.
     *
     * 이 Task는 적용한 Effect를 Handle로 제거해야 하므로
     * Infinite 또는 Duration 타입의 GE를 사용하는 것을 전제로 합니다.
     */
    UPROPERTY(EditAnywhere, Category = "Parameter")
    TSubclassOf<UGameplayEffect> GameplayEffectClass;

    /** 적용할 GameplayEffect의 레벨입니다. */
    UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
    float EffectLevel = 1.0f;

    /** GE가 실제로 적용된 ASC입니다. */
    UPROPERTY(Transient)
    TObjectPtr<UAbilitySystemComponent> AppliedAbilitySystemComponent = nullptr;

    /** 적용한 GE를 정확히 제거하기 위한 Handle입니다. */
    UPROPERTY(Transient)
    FActiveGameplayEffectHandle ActiveEffectHandle;
};


/**
 * 현재 StateTree 상태가 활성화된 동안 자신에게 GameplayEffect를 적용합니다.
 */
USTRUCT(meta = (DisplayName = "Apply Gameplay Effect While Active"))
struct EMPTYCITY_API FSTTask_ApplyGEWhileActive : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FSTTask_ApplyGEWhileActiveInstanceData;

    FSTTask_ApplyGEWhileActive();

    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
    /** StateTree 실행 Owner로부터 GE를 적용할 Actor를 찾습니다. */
    AActor* ResolveTargetActor(FStateTreeExecutionContext& Context) const;
};