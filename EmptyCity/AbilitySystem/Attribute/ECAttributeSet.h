#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "ECAttributeSet.generated.h"

struct FGameplayEffectSpec;
class UECAbilitySystemComponent;

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_MULTICAST_DELEGATE_SixParams(FECAttributeEvent, 
	AActor*						/*EffectInstigator*/, 
	AActor*						/*EffectCauser*/,
	const FGameplayEffectSpec*	/*EffectSpec*/, 
	float						/*EffectMagnitude*/,
	float						/*OldValue*/, 
	float						/*NewValue*/
	);

UCLASS()
class EMPTYCITY_API UECAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// AttributeSet Interface
// ─────────────────────────────────────────────────────────────
public:
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
	
// ─────────────────────────────────────────────────────────────
// Clamp Method
// ─────────────────────────────────────────────────────────────
protected:
	/** Attribute 값을 보정하는 함수입니다. */
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const { };
	
	
// ─────────────────────────────────────────────────────────────
// Ability Getter
// ─────────────────────────────────────────────────────────────			
public:
	UECAbilitySystemComponent* GetAbilitySystemComponent() const;
	
};
