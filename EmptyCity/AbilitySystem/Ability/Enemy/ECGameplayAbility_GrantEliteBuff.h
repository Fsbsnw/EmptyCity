// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/ECGameplayAbility.h"
#include "ECGameplayAbility_GrantEliteBuff.generated.h"

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECGameplayAbility_GrantEliteBuff : public UECGameplayAbility
{
	GENERATED_BODY()
public:
	UECGameplayAbility_GrantEliteBuff();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** EnemyNormal 부하들에게 적용할 버프 Effect입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> BuffEffectClass;
};
