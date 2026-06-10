#include "ECGameplayAbility_StaminaRegen.h"

UECGameplayAbility_StaminaRegen::UECGameplayAbility_StaminaRegen()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationPolicy = EAbilityActivationPolicy::OnSpawn;
	ActivationGroup = EAbilityActivationGroup::Independent;
}
