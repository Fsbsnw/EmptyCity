#pragma once

#include "CoreMinimal.h"
#include "ECGameplayAbility.h"
#include "ECGameplayAbility_Crouch.generated.h"

UCLASS()
class EMPTYCITY_API UECGameplayAbility_Crouch : public UECGameplayAbility
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// GameplayAbility Interface
// ─────────────────────────────────────────────────────────────
public:
	UECGameplayAbility_Crouch();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputStarted() override;
	
	
// ─────────────────────────────────────────────────────────────
// Crouch
// ─────────────────────────────────────────────────────────────
private:
	/** 캐릭터의 Crouch를 발동하는 함수입니다. */
	void CharacterCrouchStart();
	
	/** 캐릭터의 Crouch를 종료하는 함수입니다. */
	void CharacterCrouchStop();
	
};