#include "ECAttributeSet.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"

void UECAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	ClampAttribute(Attribute, NewValue);
}

void UECAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	ClampAttribute(Attribute, NewValue);
}

UECAbilitySystemComponent* UECAttributeSet::GetAbilitySystemComponent() const
{
	return Cast<UECAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
