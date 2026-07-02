// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/ECGameplayAbility.h"
#include "ECGameplayAbility_EnemyAttack.generated.h"

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECGameplayAbility_EnemyAttack : public UECGameplayAbility
{
	GENERATED_BODY()

public:
	UECGameplayAbility_EnemyAttack();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** 전방 스캔을 수행하여 타겟들을 찾습니다. */
	void ExecuteHitCheck();
	
	/** 찾아낸 타겟에게 데미지(GE)를 적용합니다. */
	void HandleMeleeHit(AActor* TargetActor, const FHitResult& HitResult);

	/** GameplayCue에 전달할 무기 타입을 정의합니다. */
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag WeaponTypeTag;

	/** GameplayCue에 전달할 공격 타입을 정의합니다. */
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag AttackTypeTag;
	
protected:
	/** 에디터 상에서 Debug Sphere를 그릴지 여부 */
	UPROPERTY(EditDefaultsOnly, Category = "EnemyAttack|Debug")
	bool bDrawDebug = true;
	
// ─────────────────────────────────────────────────────────────
// ECHealthSet 데미지 정보
// ─────────────────────────────────────────────────────────────
	/** 적용할 데미지 (GE_Damage) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyAttack|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 기본 데미지 수치 */
	UPROPERTY(EditDefaultsOnly, Category = "EnemyAttack|Damage")
	FScalableFloat Damage;

	/** 공격 판정 반경 (Sphere Radius) */
	UPROPERTY(EditDefaultsOnly, Category = "EnemyAttack|Range")
	float AttackRadius = 150.f;

	/** 공격 판정 중심점을 액터 위치 기준 앞쪽으로 이동 */
	UPROPERTY(EditDefaultsOnly, Category = "EnemyAttack|Range")
	float AttackForwardOffset = 100.f;
	
// ─────────────────────────────────────────────────────────────
// ECStaminaSet 데미지 정보
// ─────────────────────────────────────────────────────────────
	/** 적용할 스태미나 데미지 (GE_Stamina_Damage) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyAttack|Stamina")
	TSubclassOf<UGameplayEffect> StaminaDamageEffectClass;

	/** 기본 스태미나 데미지 수치 */
	UPROPERTY(EditDefaultsOnly, Category = "EnemyAttack|Stamina")
	FScalableFloat StaminaDamage;
};
