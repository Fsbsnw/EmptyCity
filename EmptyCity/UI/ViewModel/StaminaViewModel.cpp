// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/StaminaViewModel.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Attribute/ECStaminaSet.h"

void UStaminaViewModel::BindCallbacksToDependencies(AActor* ContextActor)
{
	if (!ContextActor)
	{
		return;
	}
	
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ContextActor);

	if (ASC)
	{
		const UECStaminaSet* StaminaSet = ASC->GetSet<UECStaminaSet>();
		if (StaminaSet)
		{
			StaminaSet->OnStaminaChanged.AddUObject(this, &ThisClass::HandleStaminaChanged);
			StaminaSet->OnMaxStaminaChanged.AddUObject(this, &ThisClass::HandleMaxStaminaChanged);
			
			SetCurrentStamina(FMath::RoundToInt(StaminaSet->GetStamina()));
			SetMaxStamina(FMath::RoundToInt(StaminaSet->GetMaxStamina()));
		}
	}
}

void UStaminaViewModel::BroadcastInitialValues()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentStamina);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(MaxStamina);
}

void UStaminaViewModel::SetCurrentStamina(int32 NewStamina)
{
	// 변화가 있으면 true, 없으면 false 반환
	if (UE_MVVM_SET_PROPERTY_VALUE(CurrentStamina, NewStamina))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStaminaPercent);
	}
}

void UStaminaViewModel::SetMaxStamina(int32 NewMaxStamina)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxStamina, NewMaxStamina);
}

float UStaminaViewModel::GetStaminaPercent() const
{
	if (MaxStamina <= 0)
	{
		return 0.0f; // 최대 스태미나가 0이하면 0% 반환
	}
    
	// float으로 캐스팅하여 소수점 비율 계산
	return static_cast<float>(CurrentStamina) / static_cast<float>(MaxStamina);
}

void UStaminaViewModel::HandleStaminaChanged(AActor* EffectInstigator, AActor* EffectCauser,
	const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue)
{
	SetCurrentStamina(FMath::RoundToInt(NewValue));
}

void UStaminaViewModel::HandleMaxStaminaChanged(AActor* EffectInstigator, AActor* EffectCauser,
	const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue)
{
	SetMaxStamina(FMath::RoundToInt(NewValue));
}
