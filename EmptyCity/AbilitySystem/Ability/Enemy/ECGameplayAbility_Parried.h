// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/ECGameplayAbility.h"
#include "ECGameplayAbility_Parried.generated.h"

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECGameplayAbility_Parried : public UECGameplayAbility
{
	GENERATED_BODY()	
public:
	UECGameplayAbility_Parried();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	/** 패링을 당할 때 실행할 애니메이션 몽타주입니다. */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> ParriedReactionMontage;
private:
	UFUNCTION()
	void OnParriedReactionFinished();

	UFUNCTION()
	void OnParriedReactionCanceled();
};
