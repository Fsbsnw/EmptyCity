#include "ECGameplayAbility_Guard.h"
#include "ECGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Equipment/Weapon/ECWeaponInstance.h"

namespace GuardMontageSection
{
	const FName Start(TEXT("Start"));
	const FName Loop(TEXT("Loop"));
	const FName End(TEXT("End"));
}

UECGameplayAbility_Guard::UECGameplayAbility_Guard()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationPolicy = EAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = EAbilityActivationGroup::Exclusive_Blocking;
}

void UECGameplayAbility_Guard::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UE_LOG(LogTemp, Warning, TEXT("가드"));
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		K2_EndAbility();
		return;
	}
	
	UECWeaponInstance* WeaponInstance = Cast<UECWeaponInstance>(GetAssociatedEquipment());
	if (!WeaponInstance)
	{
		K2_EndAbility();
		return;
	}
	
	ActiveGuardMontage = WeaponInstance->GuardMontage;
	if (!ActiveGuardMontage)
	{
		K2_EndAbility();
		return;
	}
		
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ActiveGuardMontage, 1.0f, GuardMontageSection::Start);
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageTask->ReadyForActivation();
	
	UECAbilitySystemComponent* ASC = GetECAbilitySystemComponentFromActorInfo();
	ASC->SetLooseGameplayTagCount(ECGameplayTags::Status_Guarding, 1);
}

void UECGameplayAbility_Guard::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	
	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	if (!AnimInstance)
	{
		K2_EndAbility();
		return;
	}
	
	AnimInstance->Montage_JumpToSection(GuardMontageSection::End, ActiveGuardMontage);
	K2_EndAbility();
}

void UECGameplayAbility_Guard::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UECAbilitySystemComponent* ASC = GetECAbilitySystemComponentFromActorInfo())
	{
		ASC->SetLooseGameplayTagCount(ECGameplayTags::Status_Guarding, 0);

		// 몽타주가 Notify 구간 도중 강제 종료되더라도 패링 태그가 남지 않게 보장합니다.
		ASC->SetLooseGameplayTagCount(ECGameplayTags::Status_ParryWindow, 0);
	}

	ActiveGuardMontage = nullptr;
		
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
