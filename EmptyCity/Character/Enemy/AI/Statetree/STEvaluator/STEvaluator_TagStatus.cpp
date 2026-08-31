#include "STEvaluator_TagStatus.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"

void FSTEvaluator_TagStatus::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!IsValid(InstanceData.AIController) || !InstanceData.StatusTag.IsValid())
	{
		return;
	}

	APawn* ControlledPawn = InstanceData.AIController->GetPawn();
	if (!IsValid(ControlledPawn))
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlledPawn);
	if (!IsValid(ASC))
	{
		return;
	}

	const bool bCurrentState = ASC->HasMatchingGameplayTag(InstanceData.StatusTag);

	// State Tree에 실제로 바인딩 된 변수를 가져옵니다.
	bool* StatusValue = InstanceData.StatusRef.GetMutablePtr(Context);
	if (StatusValue == nullptr)
	{
		return;
	}

	*StatusValue = bCurrentState;

	// 최초 틱에는 현재 상태만 저장하고 변경 이벤트를 보내지 않습니다.
	if (!InstanceData.bInitialized)
	{
		InstanceData.bPreviousState = bCurrentState;
		InstanceData.bInitialized = true;
		return;
	}

	if (bCurrentState == InstanceData.bPreviousState)
	{
		return;
	}

	if (InstanceData.StatusChangedEventTag.IsValid())
	{
		Context.SendEvent(InstanceData.StatusChangedEventTag);
	}

	InstanceData.bPreviousState = bCurrentState;
}