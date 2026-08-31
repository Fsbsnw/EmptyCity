#include "ECGameplayAbility_Crouch.h"
#include "ECGameplayTags.h"
#include "Character/ECCharacterBase.h"
#include "Character/ECCharacterMovementComponent.h"
#include "GameFramework/Character.h"

UECGameplayAbility_Crouch::UECGameplayAbility_Crouch()
{
	AbilityTags.AddTag(ECGameplayTags::Ability_Type_Action_Crouch);
	ActivationPolicy = EAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = EAbilityActivationGroup::Exclusive_Replaceable;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UECGameplayAbility_Crouch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CharacterCrouchStart();
}

void UECGameplayAbility_Crouch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	CharacterCrouchStop();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UECGameplayAbility_Crouch::InputStarted()
{
	Super::InputStarted();
	
	if (AECCharacterBase* PSCharacter = GetECCharacterFromActorInfo())
	{
		if (PSCharacter->bIsCrouched)
		{
			K2_EndAbility();
		}
	}
}

void UECGameplayAbility_Crouch::CharacterCrouchStart()
{
	if (AECCharacterBase* PSCharacter = GetECCharacterFromActorInfo())
	{
		if (const UECCharacterMovementComponent* MoveComp = Cast<UECCharacterMovementComponent>(PSCharacter->GetCharacterMovement()))
		{
			if (MoveComp->IsMovingOnGround())
			{
				PSCharacter->Crouch();
			}
		}
	}
}

void UECGameplayAbility_Crouch::CharacterCrouchStop()
{
	if (AECCharacterBase* PSCharacter = GetECCharacterFromActorInfo())
	{
		if (const UECCharacterMovementComponent* MoveComp = Cast<UECCharacterMovementComponent>(PSCharacter->GetCharacterMovement()))
		{
			if (PSCharacter->bIsCrouched || MoveComp->bWantsToCrouch)
			{
				PSCharacter->UnCrouch();
			}
		}
	}
}
