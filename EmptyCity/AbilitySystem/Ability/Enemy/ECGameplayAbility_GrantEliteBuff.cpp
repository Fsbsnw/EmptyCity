// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/Enemy/ECGameplayAbility_GrantEliteBuff.h"

#include "AbilitySystemGlobals.h"
#include "ECGameplayTags.h"
#include "Character/Enemy/ECEnemyNormal.h"

UECGameplayAbility_GrantEliteBuff::UECGameplayAbility_GrantEliteBuff()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = ECGameplayTags::Ability_Skill_GrantEliteBuff;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    
	AbilityTriggers.Add(TriggerData);
}

void UECGameplayAbility_GrantEliteBuff::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();  
    
	// ASC가 없거나, GE 클래스가 비어있거나, 전달받은 이벤트 데이터가 없으면 즉시 종료
	if (!SourceASC || !BuffEffectClass || !TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	FGameplayEffectSpecHandle BuffSpecHandle = MakeOutgoingGameplayEffectSpec(BuffEffectClass, GetAbilityLevel());
	if (!BuffSpecHandle.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	BuffSpecHandle.Data->GetContext().AddSourceObject(this);

	// Payload에서 TargetData를 꺼냅니다.
	if (TriggerEventData->TargetData.Data.IsValidIndex(0))
	{
		// 부하 액터들을 가져옵니다.
		TArray<TWeakObjectPtr<AActor>> TargetActors = TriggerEventData->TargetData.Data[0]->GetActors();

		for (TWeakObjectPtr<AActor> WeakTarget : TargetActors)
		{
			if (AActor* TargetActor = WeakTarget.Get())
			{
				UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
				if (TargetASC)
				{
					// Buff 적용
					SourceASC->ApplyGameplayEffectSpecToTarget(*BuffSpecHandle.Data.Get(), TargetASC);
				}
			}
		}
	}

	// @TODO: 버프 애니메이션 몽타주 연계?
    
	// 로직이 모두 끝나면 어빌리티 정상 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
