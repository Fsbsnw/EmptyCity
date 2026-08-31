#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitReactionComponent.generated.h"

class UECAbilitySystemComponent;
class UAbilitySystemComponent;
class UECHealthSet;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EMPTYCITY_API UHitReactionComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UHitReactionComponent();
	
	void InitializeWithAbilitySystem(UECAbilitySystemComponent* InASC);

// ─────────────────────────────────────────────────────────────
// Cache Variable
// ─────────────────────────────────────────────────────────────
protected:
	/** Owner의 AbilitySystemComponent를 캐싱해놓은 변수입니다. */
	UPROPERTY()
	TObjectPtr<UECAbilitySystemComponent> ASC;
	
// ─────────────────────────────────────────────────────────────
// Hit Stop
// ─────────────────────────────────────────────────────────────
public:
	/** 데미지 처리 구간에서 히트 스탑을 처리합니다. */
	void PlayHitStop(float TimeDuration = 0.3f, float TimeDilation = 0.05f);

	UPROPERTY(EditAnywhere) // 블루프린트 테스트용
	bool bCanPlayHitStop = true;

private:
	/** 히트 스탑이 완료된 후에 딜레이를 원상복구합니다. */
	void RestoreTimeDilation();

	/** 다단 히트 시 타이머를 덮어씌우기 위한 타이머 핸들 */
	FTimerHandle HitStopTimerHandle;

// ─────────────────────────────────────────────────────────────
// Hit Reaction Animation
// ─────────────────────────────────────────────────────────────
public:
	/** 공격이 날아온 위치를 기준으로 방향별 피격 몽타주를 재생합니다. */
	void PlayWeakHitReaction(const FVector& HitSourceLocation = FVector::ZeroVector);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Combat | Hit Reaction")
	TObjectPtr<UAnimMontage> HitReactionMontage;

// ─────────────────────────────────────────────────────────────
// Knockback
// ─────────────────────────────────────────────────────────────
public:
	/** 넉백 정보를 받아서 넉백 상태를 활성화시킵니다. */
	void ApplyGroundKnockback(const FVector& Direction, float InitialSpeed, float Duration);

	/** 넉백을 적용합니다. */
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** 넉백 상태를 종료시킵니다. */
	void StopGroundKnockback();

	FVector KnockbackDirection = FVector::ZeroVector;
	float KnockbackInitialSpeed = 0.0f;
	float KnockbackDuration = 0.0f;
	float KnockbackElapsedTime = 0.0f;
	bool bGroundKnockbackActive = false;
};
