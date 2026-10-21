// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/HealthViewModel.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Attribute/ECHealthSet.h"
#include "Character/Enemy/ECEnemyCharacterBase.h"
#include "GameplayEffectTypes.h"
#include "Subsystem/ECEnemyDataSubsystem.h"

void UHealthViewModel::BindCallbacksToDependencies(AActor* ContextActor)
{
	UnbindAttributeDelegates();
	Super::BindCallbacksToDependencies(ContextActor);

	if (ContextActor && ContextActor->IsA<AECEnemyCharacterBase>())
	{
		SetEnemyName(UECEnemyDataSubsystem::Get(ContextActor).FindEnemyDisplayName(ContextActor->GetClass()));
	}
	else
	{
		SetEnemyName(FText::GetEmpty());
	}
	
	UAbilitySystemComponent* ASC = ContextActor
		? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ContextActor)
		: nullptr;
	if (!ASC)
	{
		ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetPlayerPawn());
	}

	if (ASC)
	{
		const UECHealthSet* HealthSet = ASC->GetSet<UECHealthSet>();
		if (HealthSet)
		{
			BoundAbilitySystemComponent = ASC;
			HealthChangedDelegateHandle = ASC
				->GetGameplayAttributeValueChangeDelegate(UECHealthSet::GetHealthAttribute())
				.AddUObject(this, &ThisClass::HandleHealthChanged);
			MaxHealthChangedDelegateHandle = ASC
				->GetGameplayAttributeValueChangeDelegate(UECHealthSet::GetMaxHealthAttribute())
				.AddUObject(this, &ThisClass::HandleMaxHealthChanged);
			
			SetCurrentHealth(FMath::RoundToInt(HealthSet->GetHealth()));
			SetMaxHealth(FMath::RoundToInt(HealthSet->GetMaxHealth()));
		}
	}
}

void UHealthViewModel::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	// 아무 조건 없이 값의 현재 상태를 알립니다.
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentHealth);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(MaxHealth);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(EnemyName);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
}

void UHealthViewModel::BeginDestroy()
{
	UnbindAttributeDelegates();
	Super::BeginDestroy();
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
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxHealth, NewMaxHealth))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	}
}

void UHealthViewModel::SetEnemyName(const FText& NewEnemyName)
{
	UE_MVVM_SET_PROPERTY_VALUE(EnemyName, NewEnemyName);
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

void UHealthViewModel::HandleHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	SetCurrentHealth(FMath::RoundToInt(ChangeData.NewValue));
}

void UHealthViewModel::HandleMaxHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	SetMaxHealth(FMath::RoundToInt(ChangeData.NewValue));
}

void UHealthViewModel::UnbindAttributeDelegates()
{
	if (UAbilitySystemComponent* ASC = BoundAbilitySystemComponent.Get())
	{
		if (HealthChangedDelegateHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UECHealthSet::GetHealthAttribute())
				.Remove(HealthChangedDelegateHandle);
		}
		if (MaxHealthChangedDelegateHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UECHealthSet::GetMaxHealthAttribute())
				.Remove(MaxHealthChangedDelegateHandle);
		}
	}

	BoundAbilitySystemComponent.Reset();
	HealthChangedDelegateHandle.Reset();
	MaxHealthChangedDelegateHandle.Reset();
}
