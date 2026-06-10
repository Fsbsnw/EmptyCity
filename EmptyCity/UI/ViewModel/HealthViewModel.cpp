// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/HealthViewModel.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Attribute/ECHealthSet.h"

void UHealthViewModel::BindCallbacksToDependencies(AActor* ContextActor)
{
	if (!ContextActor)
	{
		return;
	}
	
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ContextActor);

	if (ASC)
	{
		const UECHealthSet* HealthSet = ASC->GetSet<UECHealthSet>();
		if (HealthSet)
		{
			HealthSet->OnHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);
			HealthSet->OnMaxHealthChanged.AddUObject(this, &ThisClass::HandleMaxHealthChanged);
			
			SetCurrentHealth(FMath::RoundToInt(HealthSet->GetHealth()));
			SetMaxHealth(FMath::RoundToInt(HealthSet->GetMaxHealth()));
		}
	}
}

void UHealthViewModel::BroadcastInitialValues()
{
	// 아무 조건 없이 값의 현재 상태를 알립니다.
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentHealth);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(MaxHealth);
}


void UHealthViewModel::SetCurrentHealth(int32 NewHealth)
{
	// 현재(CurrentHealth) 값과 새로운(NewHealth) 값을 비교해서 변화가 있으면 알립니다.
	// 변화가 있으면 true, 없으면 false 반환
	if (UE_MVVM_SET_PROPERTY_VALUE(CurrentHealth, NewHealth))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	}
}

void UHealthViewModel::SetMaxHealth(int32 NewMaxHealth)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxHealth, NewMaxHealth);
}

float UHealthViewModel::GetHealthPercent() const
{
	if (MaxHealth <= 0)
	{
		return 0.0f; // 최대 체력이 0이하면 0% 반환
	}
    
	// float으로 캐스팅하여 소수점 비율 계산
	return static_cast<float>(CurrentHealth) / static_cast<float>(MaxHealth);
}

void UHealthViewModel::HandleHealthChanged(AActor* EffectInstigator, AActor* EffectCauser,
	const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue)
{
	SetCurrentHealth(FMath::RoundToInt(NewValue));
}

void UHealthViewModel::HandleMaxHealthChanged(AActor* EffectInstigator, AActor* EffectCauser,
	const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue)
{
	SetMaxHealth(FMath::RoundToInt(NewValue));
}