// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/Enemy/ECEnemyDamageAbility_Melee.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ECGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Enemy/Component/EnemyMeleeTraceComponent.h"
#include "Equipment/Weapon/WeaponTrailComponent.h"

UECEnemyDamageAbility_Melee::UECEnemyDamageAbility_Melee()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	AbilityTags.AddTag(ECGameplayTags::Ability_Type_Action_Attack_Melee);
}

void UECEnemyDamageAbility_Melee::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	bIsParried = false;

	// 종료 직전에 트레이싱을 종료합니다.
	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		if (UEnemyMeleeTraceComponent* TraceComponent = AvatarActor->FindComponentByClass<UEnemyMeleeTraceComponent>())
		{
			TraceComponent->EndTrace();
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UECEnemyDamageAbility_Melee::SetupAttackTasks()
{
	UAbilityTask_WaitGameplayEvent* HitEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ECGameplayTags::GameplayEvent_Attack_Melee_Hit);

	HitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnAttackHit);
	HitEventTask->ReadyForActivation();
}

void UECEnemyDamageAbility_Melee::OnAttackHit(FGameplayEventData EventData)
{
	if (EventData.TargetData.Num() == 0)
	{
		return;
	}

	const FHitResult* HitResult = EventData.TargetData.Get(0)->GetHitResult();
	if (!HitResult)
	{
		return;
	}

	HandleMeleeHit(*HitResult);
}

void UECEnemyDamageAbility_Melee::OnMontageCancelled()
{
	if (bIsParried)
	{
		AActor* AvatarActor = GetAvatarActorFromActorInfo();
		
		if (AvatarActor)
		{
			if (UEnemyMeleeTraceComponent* TraceComponent = AvatarActor->FindComponentByClass<UEnemyMeleeTraceComponent>())
			{
				TraceComponent->EndTrace();
			}

			if (UWeaponTrailComponent* TrailComponent = AvatarActor->FindComponentByClass<UWeaponTrailComponent>())
			{
				TrailComponent->ForceEndTrail();
			}
		}
		// GA_Parried가 실행되면서 Melee Attack GA 종료를 위임시킵니다.
		return;
	}
	Super::OnMontageCancelled();
}

void UECEnemyDamageAbility_Melee::HandleMeleeHit(const FHitResult& HitResult)
{
	AActor* TargetActor = HitResult.GetActor();
	if (!TargetActor)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);

	if (!SourceASC || !TargetASC)
	{
		return;
	}

	AActor* SourceActor = GetAvatarActorFromActorInfo();
	if (!SourceActor)
	{
		return;
	}

	// 패링 가능 구간인지 확인합니다. 
	if (TargetASC->HasMatchingGameplayTag(ECGameplayTags::Status_ParryWindow))
	{
		const FVector TargetToAttacker = (SourceActor->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal2D();
		const FVector TargetForward = TargetActor->GetActorForwardVector().GetSafeNormal2D();

		const float Dot = FVector::DotProduct(TargetForward, TargetToAttacker);

		// 대략 전방 120도 이내
		if (Dot >= 0.5f)
		{
			bIsParried = true;
			
			FGameplayEventData EventData;
			EventData.Instigator = TargetActor; // 패링한 플레이어
			EventData.Target = SourceActor;     // 패링당한 적
			EventData.ContextHandle.AddHitResult(HitResult);

			// 공격자에게 패링 이벤트 전달
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
				SourceActor,
				ECGameplayTags::GameplayEvent_Parried,
				EventData);

			// 방어자에게 패링 성공 이벤트 전달
			EventData.Instigator = SourceActor;
			EventData.Target = TargetActor;

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
				TargetActor,
				ECGameplayTags::GameplayEvent_ParrySuccess,
				EventData);

			return;
		}
	}

	if (ApplyDamageToTarget(TargetActor, &HitResult))
	{
		SendKnockbackEvent(TargetActor, HitResult);
	}
}

void UECEnemyDamageAbility_Melee::SendKnockbackEvent(AActor* TargetActor, const FHitResult& HitResult) const
{
	if (!bApplyKnockback || KnockbackSpeed <= 0.0f || !TargetActor)
	{
		return;
	}

	AActor* SourceActor = GetAvatarActorFromActorInfo();
	if (!SourceActor)
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = ECGameplayTags::GameplayEvent_HitReaction_Knockback;
	EventData.Instigator = SourceActor;
	EventData.Target = TargetActor;
	EventData.EventMagnitude = KnockbackSpeed;
	EventData.ContextHandle.AddHitResult(HitResult);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, EventData.EventTag,EventData);
}