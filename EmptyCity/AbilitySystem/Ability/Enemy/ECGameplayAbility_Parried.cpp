// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/Enemy/ECGameplayAbility_Parried.h"

#include "ECGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"

UECGameplayAbility_Parried::UECGameplayAbility_Parried()
{
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = ECGameplayTags::GameplayEvent_Parried;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UECGameplayAbility_Parried::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Melee Attack 몽타주를 실행시킵니다.
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ParriedReactionMontage, 1.f);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnParriedReactionFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnParriedReactionCanceled);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnParriedReactionCanceled);
	MontageTask->ReadyForActivation();
}

void UECGameplayAbility_Parried::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	// 패링 애니메이션이 종료되면서 이전에 실행중이었던 Attack GA를 종료시키고 State Tree에서 상태 전환을 실행합니다.
	FGameplayTagContainer AbilityToCancel(ECGameplayTags::Ability);
	GetECAbilitySystemComponentFromActorInfo()->CancelAbilities(&AbilityToCancel);
}

void UECGameplayAbility_Parried::OnParriedReactionFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UECGameplayAbility_Parried::OnParriedReactionCanceled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}