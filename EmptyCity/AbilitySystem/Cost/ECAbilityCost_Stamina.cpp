#include "ECAbilityCost_Stamina.h"
#include "AbilitySystem/Ability/ECGameplayAbility.h"
#include "AbilitySystem/Attribute/ECStaminaSet.h"

UECAbilityCost_Stamina::UECAbilityCost_Stamina()
{
	Amount.SetValue(10.f);
}

bool UECAbilityCost_Stamina::CheckCost(const UECGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		return false;
	}
	
	// 스태미나를 0 초과로 가지고 있어야 발동 가능
	const float CurrentStamina = ASC->GetNumericAttribute(UECStaminaSet::GetStaminaAttribute());
	if (CurrentStamina > 0)
	{
		return true;
	}

	if (OptionalRelevantTags && FailureTag.IsValid())
	{
		OptionalRelevantTags->AddTag(FailureTag);
	}
	
	return false;
}

void UECAbilityCost_Stamina::ApplyCost(const UECGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC || !CostEffect)
	{
		return;
	}

	const int32 Level = Ability->GetAbilityLevel(Handle, ActorInfo);
	const float Cost = Amount.GetValueAtLevel(Level);

	// SetByCaller 로 StaminaCost 값을 전달 → StaminaSet 의 StaminaCost 처리 파이프라인 사용
	FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
	Ctx.AddInstigator(ActorInfo->OwnerActor.Get(), ActorInfo->AvatarActor.Get());

	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(CostEffect, Level, Ctx);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("SetByCaller.Stamina")), Cost);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
	}
}
