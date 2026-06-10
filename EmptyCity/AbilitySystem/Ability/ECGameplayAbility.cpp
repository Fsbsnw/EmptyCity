#include "ECGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "ECLogChannels.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "AbilitySystem/Cost/ECAbilityCost.h"
#include "Character/ECCharacterBase.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "GameplayEffect.h"

UECGameplayAbility::UECGameplayAbility()
{
	ActivationPolicy = EAbilityActivationPolicy::OnInputTriggered;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UECGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// 부여되는 즉시, OnSpawn 정책이라면 자동 발동을 시도합니다.
	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UECGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UECAbilitySystemComponent* ASC = GetECAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	// PeriodicEffects에 등록된 GE를 순차적으로 적용하고 핸들을 저장합니다.
	for (const TSubclassOf<UGameplayEffect>& EffectClass : PeriodicEffects)
	{
		if (!EffectClass) continue;

		const FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), Ctx);
		if (SpecHandle.IsValid())
		{
			AppliedPeriodicEffectHandles.Add(ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get()));
		}
	}
}

void UECGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UECAbilitySystemComponent* ASC = GetECAbilitySystemComponentFromActorInfo())
	{
		// 활성화 중 적용된 모든 Periodic GE를 일괄 제거합니다.
		for (FActiveGameplayEffectHandle& EffectHandle : AppliedPeriodicEffectHandles)
		{
			if (EffectHandle.IsValid())
			{
				ASC->RemoveActiveGameplayEffect(EffectHandle);
			}
		}
	}
	AppliedPeriodicEffectHandles.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UECGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	UECAbilitySystemComponent* ASC = CastChecked<UECAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (ASC->IsActivationGroupBlocked(ActivationGroup))
	{
		return false;
	}

	return true;
}

void UECGameplayAbility::SetCanBeCanceled(bool bCanBeCanceled)
{
	if (!bCanBeCanceled && (ActivationGroup == EAbilityActivationGroup::Exclusive_Replaceable))
	{
		LOG_ABILITY(TEXT("[%s]의 ActivationGroup이 Replaceable이라 설정할 수 없습니다."), *GetName());
		return;
	}

	Super::SetCanBeCanceled(bCanBeCanceled);
}

bool UECGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) || !ActorInfo)
	{
		return false;
	}

	// 어빌리티 발동에 필요한 코스트를 소모할 수 있는지 검사합니다.
	for (const TObjectPtr<UECAbilityCost>& AdditionalCost : AdditionalCosts)
	{
		if (AdditionalCost != nullptr)
		{
			if (!AdditionalCost->CheckCost(this, Handle, ActorInfo, OptionalRelevantTags))
			{
				return false;
			}
		}
	}

	return true;
}

void UECGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

	check(ActorInfo);

	// 등록된 추가 코스트를 순회하며 각각 비용을 지불합니다.
	for (const TObjectPtr<UECAbilityCost>& AdditionalCost : AdditionalCosts)
	{
		if (AdditionalCost != nullptr)
		{
			AdditionalCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
		}
	}
}

void UECGameplayAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	if (bIsPressed)
	{
		return;
	}

	bIsPressed = true;
	InputStarted();
}

void UECGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	bIsPressed = false;
}

void UECGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
{
	if (ActorInfo && !Spec.IsActive() && (ActivationPolicy == EAbilityActivationPolicy::OnSpawn))
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

		// 아바타(캐릭터)가 안정적인 상태일 때만 발동합니다.
		if (ASC && AvatarActor && (AvatarActor->GetLifeSpan() <= 0.0f))
		{
			ASC->TryActivateAbility(Spec.Handle);
		}
	}
}

AECPlayerController* UECGameplayAbility::GetECPlayerControllerFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<AECPlayerController>(CurrentActorInfo->PlayerController.Get()) : nullptr);
}

UECAbilitySystemComponent* UECGameplayAbility::GetECAbilitySystemComponentFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<UECAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr);
}

AECCharacterBase* UECGameplayAbility::GetECCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<AECCharacterBase>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}
