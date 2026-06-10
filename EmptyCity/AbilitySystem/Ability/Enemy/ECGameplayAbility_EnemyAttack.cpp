// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/Enemy/ECGameplayAbility_EnemyAttack.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ECGameplayTags.h"
#include "Engine/OverlapResult.h"

UECGameplayAbility_EnemyAttack::UECGameplayAbility_EnemyAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	AbilityTags.AddTag(ECGameplayTags::Ability_Skill_EnemyMeleeAttack);
}

void UECGameplayAbility_EnemyAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ExecuteHitCheck();

	// 테스트용으로 Ability End, 애니메이션 몽타주 연계 필요
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UECGameplayAbility_EnemyAttack::ExecuteHitCheck()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		return;
	}

	// 캐릭터 앞쪽 위치
	const FVector Center = AvatarActor->GetActorLocation() + (AvatarActor->GetActorForwardVector() * AttackForwardOffset);

	// 디버그용 Sphere 그리기 (에디터 전용)
#if WITH_EDITOR
	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), Center, AttackRadius, 16, FColor::Red, false, 2.f);
	}
#endif

	TArray<FOverlapResult> OverlapResults;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor);

	const bool bHit = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		Center,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn), // 임시 구현, 콜리전 채널 수정 필요
		FCollisionShape::MakeSphere(AttackRadius),
		QueryParams
	);
	
	if (bHit)
	{
		// 찾은 타겟들을 순회하며 데미지 적용
		for (const FOverlapResult& HitResult : OverlapResults)
		{
			AActor* TargetActor = HitResult.GetActor();
			HandleMeleeHit(TargetActor);
		}
	}
}

void UECGameplayAbility_EnemyAttack::HandleMeleeHit(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);

	if (!SourceASC || !TargetASC || !DamageEffectClass)
	{
		return;
	}

	// EffectSpec 생성
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());

	if (!DamageSpecHandle.IsValid())
	{
		return;
	}

	// 데미지 값 설정
	float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());
	DamageSpecHandle.Data->SetSetByCallerMagnitude(ECGameplayTags::SetByCaller_Damage, ScaledDamage);
	
	// Source 정보 설정
	DamageSpecHandle.Data->GetContext().AddSourceObject(this);

	// 데미지 적용
	SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
}