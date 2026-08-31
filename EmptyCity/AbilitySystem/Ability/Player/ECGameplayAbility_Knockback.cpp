#include "AbilitySystem/Ability/Player/ECGameplayAbility_Knockback.h"

#include "ECGameplayTags.h"
#include "Character/Enemy/Component/HitReactionComponent.h"

UECGameplayAbility_Knockback::UECGameplayAbility_Knockback()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationPolicy = EAbilityActivationPolicy::ByGameplayEvent;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(ECGameplayTags::Ability_Type_Action_HitReaction_Knockback);
	SetAssetTags(AssetTags);

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = ECGameplayTags::GameplayEvent_HitReaction_Knockback;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UECGameplayAbility_Knockback::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo || !ActorInfo->IsNetAuthority() || !TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* TargetActor = GetAvatarActorFromActorInfo();
	const AActor* Instigator = TriggerEventData->Instigator;
	const float KnockbackSpeed = TriggerEventData->EventMagnitude;

	if (!TargetActor || !Instigator || KnockbackSpeed <= 0.0f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector KnockbackDirection = Instigator->GetActorForwardVector().GetSafeNormal2D();
	if (!KnockbackDirection.IsNearlyZero())
	{
		if (UHitReactionComponent* HitReaction = TargetActor->FindComponentByClass<UHitReactionComponent>())
		{
			HitReaction->ApplyGroundKnockback(KnockbackDirection, KnockbackSpeed, KnockbackDuration);
		}
	}

	// 실제 시간 기반 이동은 HitReactionComponent가 담당하므로 요청 후 즉시 종료합니다.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
