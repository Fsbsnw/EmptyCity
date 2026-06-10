#include "ECGameplayAbility_Death.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "Component/Attribute/ECHealthComponent.h"
#include "ECGameplayTags.h"
#include "TimerManager.h"
#include "Engine/World.h"

UECGameplayAbility_Death::UECGameplayAbility_Death()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// 고정되는 값이므로 CDO일 때 진행합니다.
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = ECGameplayTags::GameplayEvent_Death;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UECGameplayAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 발동 중인 다른 어빌리티들을 모두 취소합니다.
	UECAbilitySystemComponent* ASC = CastChecked<UECAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	ASC->CancelAbilities(nullptr, nullptr, this);
	
	// Death 어빌리티가 취소되지 않게 잠금 상태로 전환합니다.
	SetCanBeCanceled(false);
	
	// 사망 상태로 전환합니다.
	StartDeath();
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UECGameplayAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	check(ActorInfo);
	
	FinishDeath();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UECGameplayAbility_Death::StartDeath()
{
	if (UECHealthComponent* HealthComponent = UECHealthComponent::FindHealthComponent(GetAvatarActorFromActorInfo()))
	{
		if (HealthComponent->GetDeathState() == EDeathState::NotDead)
		{
			HealthComponent->StartDeath();

			// 2초 후 FinishDeath를 호출합니다.
			GetWorld()->GetTimerManager().SetTimer(DeathTimerHandle, this, &ThisClass::FinishDeath, 2.0f, false);
		}
	}
}

void UECGameplayAbility_Death::FinishDeath()
{
	if (UECHealthComponent* HealthComponent = UECHealthComponent::FindHealthComponent(GetAvatarActorFromActorInfo()))
	{
		if (HealthComponent->GetDeathState() == EDeathState::DeathStarted)
		{
			HealthComponent->FinishDeath();
		}
	}
}
