#include "ECGameplayAbility_Sprint.h"
#include "ECGameplayTags.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "AbilitySystem/Attribute/ECMoveSpeedSet.h"
#include "AbilitySystem/Attribute/ECStaminaSet.h"
#include "Character/ECCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

UECGameplayAbility_Sprint::UECGameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationPolicy = EAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = EAbilityActivationGroup::Exclusive_Replaceable;

	ActivationOwnedTags.AddTag(ECGameplayTags::Status_Sprinting);
}

bool UECGameplayAbility_Sprint::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
		return false;

	const AECCharacterBase* Char = Cast<AECCharacterBase>(ActorInfo->AvatarActor.Get());
	if (!Char) return false;

	const UCharacterMovementComponent* CMC = Char->GetCharacterMovement();
	
	// 지면 위가 아니면 중단합니다.
	return CMC && (CMC->IsMovingOnGround());
}

void UECGameplayAbility_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// Super에서 PeriodicEffects(Stamina 차감 GE 등)가 자동으로 적용됩니다.
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CharacterSprintStart();

	UECAbilitySystemComponent* ASC = GetECAbilitySystemComponentFromActorInfo();

	// Stamina가 0에 도달하면 어빌리티를 종료합니다.
	if (const UECStaminaSet* StaminaSet = ASC->GetSet<UECStaminaSet>())
	{
		OutOfStaminaDelegateHandle = StaminaSet->OnOutOfStamina.AddUObject(this, &ThisClass::OnOutOfStamina);
	}

	// Status.Moving 태그에 대한 변화 이벤트를 감지합니다.
	MoveStateChangedDelegateHandle = ASC->RegisterGameplayTagEvent(ECGameplayTags::Status_Moving, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnMoveStateChanged);
}

void UECGameplayAbility_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	CharacterSprintStop();

	UECAbilitySystemComponent* ASC = GetECAbilitySystemComponentFromActorInfo();

	// 어빌리티가 끝날 때 반드시 바인딩을 해제해줍니다 (메모리 누수 방지)
	if (const UECStaminaSet* StaminaSet = ASC->GetSet<UECStaminaSet>())
	{
		StaminaSet->OnOutOfStamina.Remove(OutOfStaminaDelegateHandle);
	}
	ASC->RegisterGameplayTagEvent(ECGameplayTags::Status_Moving, EGameplayTagEventType::NewOrRemoved).Remove(MoveStateChangedDelegateHandle);

	// Super에서 적용된 PeriodicEffects가 자동으로 제거됩니다.
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UECGameplayAbility_Sprint::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	
	K2_EndAbility();
}

void UECGameplayAbility_Sprint::CharacterSprintStart()
{
	AECCharacterBase* Char = GetECCharacterFromActorInfo();
	if (!Char) return;

	UCharacterMovementComponent* CMC = Char->GetCharacterMovement();
	if (!CMC) return;

	if (const UECAbilitySystemComponent* ASC = GetECAbilitySystemComponentFromActorInfo())
	{
		CMC->MaxWalkSpeed = ASC->GetNumericAttribute(UECMoveSpeedSet::GetSprintSpeedAttribute());
	}
}

void UECGameplayAbility_Sprint::CharacterSprintStop()
{
	AECCharacterBase* Char = GetECCharacterFromActorInfo();
	if (!Char) return;

	UCharacterMovementComponent* CMC = Char->GetCharacterMovement();
	if (!CMC) return;

	if (const UECAbilitySystemComponent* ASC = GetECAbilitySystemComponentFromActorInfo())
	{
		CMC->MaxWalkSpeed = ASC->GetNumericAttribute(UECMoveSpeedSet::GetMoveSpeedAttribute());
	}
}

void UECGameplayAbility_Sprint::OnMoveStateChanged(const FGameplayTag MoveTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		K2_EndAbility();
	}
}

void UECGameplayAbility_Sprint::OnOutOfStamina(AActor* Instigator, AActor* Causer, const FGameplayEffectSpec* Spec, float Magnitude, float OldValue, float NewValue)
{
	K2_EndAbility();
}
