// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/Enemy/ECGameplayAbility_Stun.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "ECGameplayTags.h"

UECGameplayAbility_Stun::UECGameplayAbility_Stun()
{
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = ECGameplayTags::GameplayEvent_Debuff_Stun;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
	
	// 스턴 상태 동안 Status 관련 태그를 부여합니다.(애니메이션, BT에서 연계 필요)
	ActivationOwnedTags.AddTag(ECGameplayTags::Status_Debuff_Stun);
}

void UECGameplayAbility_Stun::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 1. 기본 BaseStunDuration(0.8초) 타이머 시작 (끝나면 OnStunFinished 호출)
	GetWorld()->GetTimerManager().SetTimer(StunTimerHandle, this, &ThisClass::OnStunFinished, BaseStunDuration, false);

	// 2. 데미지를 받는 이벤트(예: Event_Damage_Taken)가 발생하는지 감지하는 태스크 실행
	UAbilityTask_WaitGameplayEvent* WaitHitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ECGameplayTags::GameplayEvent_DamageTaken);
	WaitHitTask->EventReceived.AddDynamic(this, &ThisClass::OnDamagedWhileStunned);
	WaitHitTask->ReadyForActivation();
}

void UECGameplayAbility_Stun::OnStunFinished()
{
	UE_LOG(LogTemp, Warning, TEXT("Stun Finished"));
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UECGameplayAbility_Stun::OnDamagedWhileStunned(FGameplayEventData Payload)
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();

	// 1. 현재 타이머에 남은 시간을 가져옵니다.
	float RemainingTime = TimerManager.GetTimerRemaining(StunTimerHandle);

	if (RemainingTime > 0.f)
	{
		// 2. 남은 시간에 StunExtensionDuration(0.2초)를 더합니다.
		float NewDuration = RemainingTime + StunExtensionDuration;

		// 3. 연장된 시간으로 타이머를 다시 세팅합니다.
		TimerManager.SetTimer(StunTimerHandle, this, &ThisClass::OnStunFinished, NewDuration, false);

		UE_LOG(LogTemp, Warning, TEXT("New Remaining Stun Duration : %f"), NewDuration);
	}
}
