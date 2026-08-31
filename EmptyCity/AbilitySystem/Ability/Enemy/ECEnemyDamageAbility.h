// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECGameplayTags.h"
#include "AbilitySystem/Ability/ECGameplayAbility.h"
#include "ECEnemyDamageAbility.generated.h"

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECEnemyDamageAbility  : public UECGameplayAbility
{
	GENERATED_BODY()

public:
	UECEnemyDamageAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/**
	 * 자식 클래스가 공격 방식별 AbilityTask를 등록합니다.
	 * 예: 근접 히트 이벤트 대기, 투사체 생성 이벤트 대기
	 */
	virtual void SetupAttackTasks();
	
	/** 타겟에게 데미지 GameplayEffect를 적용합니다. */
	bool ApplyDamageToTarget(AActor* TargetActor, const FHitResult* HitResult = nullptr);
	
	/** GameplayCue에 전달할 무기 타입을 정의합니다. */
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag WeaponTypeTag = ECGameplayTags::Weapon;

	/** GameplayCue에 전달할 공격 타입을 정의합니다. */
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag AttackTypeTag = ECGameplayTags::Attack;

	/** 공격에 실행할 애니메이션 몽타주입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> AttackMontage;

	/** 애니메이션 실행 속도입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "EnemyAttack|Range")
	float AttackMontageRate = 1.f;

	/**
	 * CharacterMovement removes animation Z while Walking and preserves gravity Z
	 * instead while Falling. Temporarily use Flying only when this montage has a
	 * meaningful vertical root-motion arc.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "EnemyAttack|Root Motion")
	bool bAutoEnableVerticalRootMotionMovement = true;

	/** Minimum cumulative vertical excursion required to enable vertical root motion. */
	UPROPERTY(EditDefaultsOnly,	Category = "EnemyAttack|Root Motion", meta = (ClampMin = "0.0", UIMin = "0.0", Units = "cm"))
	float VerticalRootMotionThreshold = 5.f;

	UFUNCTION()
	virtual void OnMontageFinished();

	UFUNCTION()
	virtual void OnMontageCancelled();
	
// ─────────────────────────────────────────────────────────────
// 데미지 정보
// ─────────────────────────────────────────────────────────────
	
protected:
	/** 적용할 데미지 (GE_Damage) 클래스입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyAttack|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 기본 체력 데미지 수치입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "EnemyAttack|Damage")
	float DamageMultiplier = 1.f;

	/** 기본 스태미나 데미지 수치입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "EnemyAttack|Damage")
	float StaminaDamage = 15.f;

private:
	bool HasMeaningfulVerticalRootMotion(const UAnimMontage* Montage) const;
	void EnableVerticalRootMotionMovement();
	void RestoreMovementModeAfterRootMotion(bool bWasCancelled);

	TWeakObjectPtr<class UCharacterMovementComponent> VerticalRootMotionMovementComponent;
	TEnumAsByte<EMovementMode> PreviousMovementMode = MOVE_None;
	uint8 PreviousCustomMovementMode = 0;
	bool bChangedMovementModeForVerticalRootMotion = false;
};
