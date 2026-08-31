#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StateTreeConditionBase.h"
#include "STCondition_CheckCooldown.generated.h"

class AActor;

UENUM()
enum class EAbilityCooldownState : uint8
{
	Ready UMETA(DisplayName = "Ready"),
	CoolingDown UMETA(DisplayName = "Cooling Down")
};

USTRUCT()
struct FSTCondition_CheckCooldownInstanceData
{
	GENERATED_BODY()

	/**
	 * 쿨다운을 검사할 액터입니다.
	 * 플레이어 TargetActor가 아니라 공격을 사용하는 Enemy Character를 바인딩합니다.
	 */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> AbilityOwner = nullptr;

	/** GA의 Cooldown GE가 부여하는 태그입니다. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag CooldownTag;

	/** Condition이 통과해야 하는 쿨다운 상태입니다. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	EAbilityCooldownState RequiredState = EAbilityCooldownState::Ready;
};

USTRUCT(meta = (DisplayName = "Check Ability Cooldown"))
struct EMPTYCITY_API FSTCondition_CheckCooldown : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTCondition_CheckCooldownInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};