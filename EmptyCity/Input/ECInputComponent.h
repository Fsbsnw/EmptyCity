#pragma once

#include "CoreMinimal.h"
#include "ECInputConfig.h"
#include "EnhancedInputComponent.h"
#include "ECInputComponent.generated.h"

struct FGameplayTag;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EMPTYCITY_API UECInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
	
	
// ─────────────────────────────────────────────────────────────
// Input Binding
// ─────────────────────────────────────────────────────────────
public:	
	/** InputTag에 해당하는 기본 입력 액션을 TriggerEvent에 맞게 바인딩하는 함수입니다. */
	template <class UserClass, typename FuncType>
	void BindNativeAction(const UECInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func);

	/** AbilityInputActions 전체를 Triggered/Completed로 일괄 바인딩하고 핸들을 BindHandles에 저장하는 함수입니다.
	 * BindHandles는 런타임 중 등록한 어빌리티를 제거하기 위한 용도입니다.
	 */
	template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityAction(const UECInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc);

	/** InputTag에 해당하는 UI 입력 액션을 Started TriggerEvent에 바인딩하는 함수입니다. */
	template <class UserClass, typename FuncType>
	void BindUIAction(const UECInputConfig* InputConfig, UserClass* Object, FuncType Func);

};


// ─────────────────────────────────────────────────────────────
// Inline Template Function
// ─────────────────────────────────────────────────────────────
template <class UserClass, typename FuncType>
void UECInputComponent::BindNativeAction(const UECInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func)
{
	check(InputConfig);
	
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag))
	{
		BindAction(IA, TriggerEvent, Object, Func);
	}
}

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UECInputComponent::BindAbilityAction(const UECInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc)
{
	check(InputConfig);
	
	for (const FECInputAction& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag).GetHandle();
			}
			
			if (ReleasedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle();
			}
		}
	}
}

template <class UserClass, typename FuncType>
void UECInputComponent::BindUIAction(const UECInputConfig* InputConfig, UserClass* Object, FuncType Func)
{
	check(InputConfig);
    
	for (const FECInputAction& Action : InputConfig->UIInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, Func, Action.InputTag);
		}
	}
}