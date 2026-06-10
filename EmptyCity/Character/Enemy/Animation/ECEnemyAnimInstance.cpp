// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Animation/ECEnemyAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ECGameplayTags.h"

void UECEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	AActor* OwningActor = GetOwningActor();
	
	if (!OwningActor)
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor);
	if (ASC)
	{
		// 설정한 태그 변화를 감지합니다.
		ASC->RegisterGameplayTagEvent(ECGameplayTags::Status_Debuff_Stun, EGameplayTagEventType::NewOrRemoved)
		   .AddUObject(this, &ThisClass::OnStunTagChanged);
	}
}

void UECEnemyAnimInstance::NativeUninitializeAnimation()
{
	AActor* OwningActor = GetOwningActor();
	
	if (OwningActor)
	{
		UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor);
		if (ASC)
		{
			// 설정한 태그에 등록된 델리게이트를 제거합니다.
			ASC->RegisterGameplayTagEvent(ECGameplayTags::Status_Debuff_Stun, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
		}
	}

	Super::NativeUninitializeAnimation();
}

void UECEnemyAnimInstance::OnStunTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	// 태그 개수가 0보다 크면 스턴 진행 상태, 아니면 스턴 제거 상태
	bIsStunned = (NewCount > 0);
}
