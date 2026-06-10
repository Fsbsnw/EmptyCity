#pragma once

#include "CoreMinimal.h"
#include "ECGameplayAbility.h"
#include "ECGameplayAbility_Jump.generated.h"

UCLASS()
class EMPTYCITY_API UECGameplayAbility_Jump : public UECGameplayAbility
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// GameplayAbility Interface
// ─────────────────────────────────────────────────────────────
public:
	UECGameplayAbility_Jump();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	
// ─────────────────────────────────────────────────────────────
// Jump
// ─────────────────────────────────────────────────────────────
protected:
	/** 캐릭터의 점프를 시작하는 함수입니다. */
	void CharacterJumpStart();

	/** 캐릭터의 점프를 중단하는 함수입니다. EndAbility에서 호출됩니다. */
	UFUNCTION()
	void CharacterJumpStop();
	
};
