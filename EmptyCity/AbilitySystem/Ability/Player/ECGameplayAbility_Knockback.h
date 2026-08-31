#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/ECGameplayAbility.h"
#include "ECGameplayAbility_Knockback.generated.h"

/**
 * 넉백 GameplayEvent를 받아 피격자의 HitReactionComponent에 지상 넉백을 요청합니다.
 * 이 어빌리티는 피격 가능한 캐릭터의 AbilitySet을 통해 한 번만 영구 부여합니다.
 */
UCLASS()
class EMPTYCITY_API UECGameplayAbility_Knockback : public UECGameplayAbility
{
	GENERATED_BODY()

public:
	UECGameplayAbility_Knockback();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** 지상 넉백이 감속하면서 유지되는 공통 시간입니다. */
	UPROPERTY(EditDefaultsOnly, Category="Knockback", meta=(ClampMin="0.01", Units="s"))
	float KnockbackDuration = 0.12f;
};
