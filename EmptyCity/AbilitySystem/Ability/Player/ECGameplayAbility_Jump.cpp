#include "ECGameplayAbility_Jump.h"
#include "ECGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_StartAbilityState.h"
#include "Character/ECCharacterBase.h"
#include "GameFramework/Character.h"

UECGameplayAbility_Jump::UECGameplayAbility_Jump()
{
	AbilityTags.AddTag(ECGameplayTags::Ability_Type_Action_Jump);
	ActivationPolicy = EAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = EAbilityActivationGroup::Exclusive_Blocking;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UECGameplayAbility_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	CharacterJumpStart();
	
	if (UAbilityTask_StartAbilityState* JumpingStateTask = UAbilityTask_StartAbilityState::StartAbilityState(this, FName("Jumping"), true))
	{
		// OnStateEnded와 OnStateInterrupted 발생 시 CharacterJumpStop이 호출되도록 묶어줍니다.
		JumpingStateTask->OnStateInterrupted.AddDynamic(this, &ThisClass::CharacterJumpStop);
        
		// 태스크 활성화
		JumpingStateTask->ReadyForActivation();
	}
	
	K2_EndAbility();
}

void UECGameplayAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UECGameplayAbility_Jump::CharacterJumpStart()
{
	if (AECCharacterBase* PSCharacter = GetECCharacterFromActorInfo())
	{
		if (!PSCharacter->bPressedJump)
		{
			PSCharacter->UnCrouch();
			PSCharacter->Jump();
		}
	}
}

void UECGameplayAbility_Jump::CharacterJumpStop()
{
	if (AECCharacterBase* PSCharacter = GetECCharacterFromActorInfo())
	{
		if (PSCharacter->bPressedJump)
		{
			PSCharacter->StopJumping();
		}
	}
}