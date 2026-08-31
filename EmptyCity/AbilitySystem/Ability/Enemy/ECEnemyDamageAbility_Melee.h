// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECEnemyDamageAbility.h"
#include "AbilitySystem/Ability/ECGameplayAbility.h"
#include "ECEnemyDamageAbility_Melee.generated.h"

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECEnemyDamageAbility_Melee : public UECEnemyDamageAbility
{
	GENERATED_BODY()

public:
	UECEnemyDamageAbility_Melee();

protected:
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void SetupAttackTasks() override;
	
	/** 히트된 타겟에게 데미지를 적용합니다. */
	void HandleMeleeHit(const FHitResult& HitResult);

	/** 이 공격이 적중했을 때 넉백을 적용할지 결정합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="EnemyAttack|Knockback")
	bool bApplyKnockback = false;

	/** 넉백의 초기 수평 속도입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="EnemyAttack|Knockback",
		meta=(EditCondition="bApplyKnockback", ClampMin="0.0", Units="cm/s"))
	float KnockbackSpeed = 650.0f;

private:
	bool bIsParried = false;

	/** 히트된 타겟을 이벤트로 전달받을 때 실행될 함수입니다. */
	UFUNCTION()
	void OnAttackHit(FGameplayEventData EventData);

	/** 피격자에게 넉백 반응 GameplayEvent를 전달합니다. */
	void SendKnockbackEvent(AActor* TargetActor, const FHitResult& HitResult) const;

	virtual void OnMontageCancelled() override;
};
