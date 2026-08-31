#pragma once

#include "CoreMinimal.h"
#include "Equipment/ECGameplayAbility_FromEquipment.h"
#include "ECGameplayAbility_PlayerAttack.generated.h"

/**
 * Ability의 공격 태그와 일치하는 장착 무기의 공격 정의를 찾아 몽타주를 재생합니다.
 * 현재는 몽타주 재생과 몽타주 종료에 따른 Ability 종료만 담당합니다.
 */
UCLASS()
class EMPTYCITY_API UECGameplayAbility_PlayerAttack : public UECGameplayAbility_FromEquipment
{
	GENERATED_BODY()

public:
	UECGameplayAbility_PlayerAttack();
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
	
};
