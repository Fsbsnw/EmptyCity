// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECEnemyDamageAbility_Melee.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ECGameplayAbility_BossSprintLeapSlam.generated.h"

class AECEnemyAIController;
class UCharacterMovementComponent;

/**
 * 보스의 Crouch -> 실제 AI Sprint 이동 -> Root Motion LeapSlam을 하나의 공격으로 실행합니다.
 * Sprint 구간에서는 AI MoveTo를 사용하고, LeapSlam 구간에 진입할 때만 Flying으로 전환하여
 * 수직 Root Motion이 CharacterMovement에 반영되도록 합니다.
 */
UCLASS()
class EMPTYCITY_API UECGameplayAbility_BossSprintLeapSlam : public UECEnemyDamageAbility_Melee
{
	GENERATED_BODY()

public:
	UECGameplayAbility_BossSprintLeapSlam();

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	/** 이 거리보다 가까우면 Ability 자체가 발동하지 않습니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Attack|Activation",
		meta=(ClampMin="0.0", Units="cm"))
	float MinActivationDistance = 900.0f;

	/** 이 거리보다 멀면 Ability 자체가 발동하지 않습니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Attack|Activation",
		meta=(ClampMin="0.0", Units="cm"))
	float MaxActivationDistance = 2200.0f;

	/** Sprint 중 이 거리까지 접근하면 LeapSlam 구간으로 넘어갑니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Attack|Sprint",
		meta=(ClampMin="0.0", Units="cm"))
	float LeapStartDistance = 450.0f;

	/** Sprint 구간에서 CharacterMovement에 적용할 실제 이동 속도입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Attack|Sprint",
		meta=(ClampMin="0.0", Units="cm/s"))
	float SprintSpeed = 700.0f;

	/** 길 찾기 실패나 진로 막힘으로 Sprint가 끝나지 않을 때 Ability를 취소하는 제한 시간입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Attack|Sprint",
		meta=(ClampMin="0.1", Units="s"))
	float MaxSprintDuration = 3.0f;

	/** Sprint 시작 전 보스와 목표 위치를 NavMesh에 투영할 때 사용할 검색 범위입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Attack|Navigation")
	FVector NavigationProjectionExtent = FVector(300.0f, 300.0f, 500.0f);

	/** 보스가 NavMesh에서 이 거리보다 멀리 벗어나 있으면 Sprint를 시작하지 않습니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Attack|Navigation",
		meta=(ClampMin="0.0", Units="cm"))
	float MaxStartNavProjectionDistance = 100.0f;

	/** 몽타주 구간과 타겟 거리를 확인하는 주기입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Attack|Flow",
		meta=(ClampMin="0.01", Units="s"))
	float FlowCheckInterval = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Attack|Montage")
	FName CrouchSectionName = TEXT("Crouch");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Attack|Montage")
	FName SprintSectionName = TEXT("Sprint");

	/** Sprint 포즈에서 블렌딩되어 재생될 Root Motion LeapSlam 전용 몽타주입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Attack|Montage")
	TObjectPtr<UAnimMontage> LeapSlamMontage;

	/** LeapSlam 전용 몽타주의 재생 속도입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Attack|Montage",
		meta=(ClampMin="0.01"))
	float LeapSlamMontageRate = 1.0f;

private:
	enum class EFlowPhase : uint8
	{
		None,
		Crouch,
		Sprint,
		LeapSlam
	};

	void UpdateFlow();
	bool BeginSprint();
	void BeginLeapSlam();
	void CancelFlow();
	void RestoreMovement(bool bWasCancelled);
	bool CacheRuntimeReferences(const FGameplayAbilityActorInfo* ActorInfo);
	bool HasRequiredMontages() const;

	UFUNCTION()
	void HandleStartSprintMontageCompleted();

	UFUNCTION()
	void HandleStartSprintMontageCancelled();

	UFUNCTION()
	void HandleLeapSlamMontageCompleted();

	UFUNCTION()
	void HandleLeapSlamMontageCancelled();

	TWeakObjectPtr<AActor> CachedTargetActor;
	TWeakObjectPtr<AECEnemyAIController> CachedAIController;
	TWeakObjectPtr<UCharacterMovementComponent> CachedMovementComponent;

	FTimerHandle FlowTimerHandle;
	EFlowPhase FlowPhase = EFlowPhase::None;
	TEnumAsByte<EMovementMode> PreviousMovementMode = MOVE_None;
	uint8 PreviousCustomMovementMode = 0;
	float PreviousMaxWalkSpeed = 0.0f;
	float SprintElapsedTime = 0.0f;
	bool bIssuedSprintMove = false;
	bool bSetSprintFocus = false;
	bool bChangedMovementModeForLeap = false;
	bool bEndingAbility = false;
};
