#pragma once

#include "CoreMinimal.h"
#include "ECGameplayAbility.h"
#include "ECGameplayAbility_Death.generated.h"

UCLASS()
class EMPTYCITY_API UECGameplayAbility_Death : public UECGameplayAbility
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// GameplayAbility Interface
// ─────────────────────────────────────────────────────────────		
public:
	UECGameplayAbility_Death();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	
// ─────────────────────────────────────────────────────────────
// Death State Method
// ─────────────────────────────────────────────────────────────		
protected:
	/** HealthComponent의 StartDeath를 호출합니다. */
	void StartDeath();
	
	/** HealthComponent의 FinishDeath를 호출합니다. */
	void FinishDeath();


// ─────────────────────────────────────────────────────────────
// Timer Variable
// ─────────────────────────────────────────────────────────────
protected:
	/** StartDeath 이후 FinishDeath 호출까지 대기하는 타이머 핸들입니다. */
	FTimerHandle DeathTimerHandle;

};
