#pragma once

#include "CoreMinimal.h"
#include "ECGameplayAbility.h"
#include "ECGameplayAbility_Sprint.generated.h"

struct FGameplayEffectSpec;

UCLASS()
class EMPTYCITY_API UECGameplayAbility_Sprint : public UECGameplayAbility
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// GameplayAbility Interface
// ─────────────────────────────────────────────────────────────	
public:
	UECGameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	
// ─────────────────────────────────────────────────────────────
// Sprint
// ─────────────────────────────────────────────────────────────
private:
	/** 캐릭터의 달리기를 수행하는 함수입니다. */
	void CharacterSprintStart();

	/** 캐릭터의 달리기 상태를 종료하는 함수입니다. */
	void CharacterSprintStop();

	/** 이동 상태를 감지하는 함수입니다. 이동이 종료되면 달리기 상태도 종료됩니다. */
	void OnMoveStateChanged(const FGameplayTag MoveTag, int32 NewCount);

	/** Stamina가 0에 도달했을 때 호출되어 어빌리티를 종료합니다. */
	void OnOutOfStamina(AActor* Instigator, AActor* Causer, const FGameplayEffectSpec* Spec, float Magnitude, float OldValue, float NewValue);

	/** 이동 상태를 감지하는 델리게이트입니다. */
	FDelegateHandle MoveStateChangedDelegateHandle;

	/** OnOutOfStamina 델리게이트 바인딩 핸들입니다. */
	FDelegateHandle OutOfStaminaDelegateHandle;

};
