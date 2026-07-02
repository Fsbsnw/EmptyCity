// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/Enemy/ECGameplayAbility_EnemyAttack.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ECGameplayTags.h"

UECGameplayAbility_EnemyAttack::UECGameplayAbility_EnemyAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	AbilityTags.AddTag(ECGameplayTags::Ability_Skill_EnemyMeleeAttack);
	WeaponTypeTag = ECGameplayTags::Weapon;
	AttackTypeTag = ECGameplayTags::Attack;
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
	if (!IsValid(AvatarActor)) return;

	// 공격의 시작점 (캐릭터 위치)
	const FVector StartLoc = AvatarActor->GetActorLocation();
	// 공격의 끝점 (캐릭터 앞쪽 위치)
	const FVector EndLoc = StartLoc + (AvatarActor->GetActorForwardVector() * AttackForwardOffset);

#if WITH_EDITOR
	if (bDrawDebug)
	{
		// 캡슐 형태로 궤적이 그려짐
		DrawDebugCapsule(GetWorld(), StartLoc + (EndLoc - StartLoc) * 0.5f, (EndLoc - StartLoc).Size() * 0.5f + AttackRadius, AttackRadius, FRotationMatrix::MakeFromZ(EndLoc - StartLoc).ToQuat(), FColor::Red, false, 2.f);
	}
#endif

	TArray<FHitResult> HitResults;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor);
	
	const bool bHit = GetWorld()->SweepMultiByObjectType(
	   HitResults,
	   StartLoc,
	   EndLoc,
	   FQuat::Identity,
	   FCollisionObjectQueryParams(ECC_Pawn), // 콜리전 채널 수정 필요
	   FCollisionShape::MakeSphere(AttackRadius),
	   QueryParams
	);
    
	if (bHit)
	{
		for (const FHitResult& HitResult : HitResults)
		{
			AActor* TargetActor = HitResult.GetActor();
			
			HandleMeleeHit(TargetActor, HitResult); 
		}
	}
}

void UECGameplayAbility_EnemyAttack::HandleMeleeHit(AActor* TargetActor, const FHitResult& HitResult)
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

	// @TODO: Blocking 관련 태그 구현
	// if (TargetASC->HasMatchingGameplayTag(ECGameplayTags::Status_Blocking) && StaminaDamageEffectClass)
	// {
	// 	// EffectSpec 생성
	// 	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(StaminaDamageEffectClass, GetAbilityLevel());
	//
	// 	if (!DamageSpecHandle.IsValid())
	// 	{
	// 		return;
	// 	}
	//
	// 	// 데미지 값 설정
	// 	float ScaledStaminaDamage = StaminaDamage.GetValueAtLevel(GetAbilityLevel());
	// 	DamageSpecHandle.Data->SetSetByCallerMagnitude(ECGameplayTags::SetByCaller_StaminaDamage, ScaledStaminaDamage);
	//
	// 	// Source 정보 설정
	// 	DamageSpecHandle.Data->GetContext().AddSourceObject(this);
	//
	// 	// 스태미나 데미지 적용
	// 	SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
	// 	return;
	// }

	// EffectSpec 생성
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());

	if (!DamageSpecHandle.IsValid())
	{
		return;
	}

	FGameplayTagContainer DynamicTags;
	DynamicTags.AddTag(WeaponTypeTag);
	DynamicTags.AddTag(AttackTypeTag);   

	DamageSpecHandle.Data->AppendDynamicAssetTags(DynamicTags);

	// 데미지 값 설정
	float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());
	DamageSpecHandle.Data->SetSetByCallerMagnitude(ECGameplayTags::SetByCaller_Damage, ScaledDamage);

	FGameplayEffectContextHandle ContextHandle = DamageSpecHandle.Data->GetContext();
	ContextHandle.AddHitResult(HitResult);
	
	// 데미지 적용
	SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
}