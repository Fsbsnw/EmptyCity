// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/ECGameplayAbility.h"
#include "ECGameplayAbility_Stun.generated.h"

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECGameplayAbility_Stun : public UECGameplayAbility
{
	GENERATED_BODY()	
public:
	UECGameplayAbility_Stun();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UFUNCTION()
	void OnStunFinished();

	UFUNCTION()
	void OnDamagedWhileStunned(FGameplayEventData Payload);
// ─────────────────────────────────────────────────────────────
// Timer Variable
// ─────────────────────────────────────────────────────────────
protected:
	/** Stun 시작~종료 호출까지 대기하는 타이머 핸들입니다. */
	FTimerHandle StunTimerHandle;

	/** 기본 스턴 지속 시간입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Stun")
	float BaseStunDuration = 0.8f; 

	/** 스턴 상태일 때 추가타를 맞으면 연장되는 스턴 시간입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Stun")
	float StunExtensionDuration = 0.2f; 
};
