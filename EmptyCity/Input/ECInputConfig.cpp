#include "ECInputConfig.h"

UECInputConfig::UECInputConfig(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

const UInputAction* UECInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FECInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}
	
	return nullptr;
}

const UInputAction* UECInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FECInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}
	
	return nullptr;
}

const UInputAction* UECInputConfig::FindUIInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FECInputAction& Action : UIInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}
	
	return nullptr;
}
