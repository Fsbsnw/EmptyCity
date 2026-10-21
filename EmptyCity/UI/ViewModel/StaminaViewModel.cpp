// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/StaminaViewModel.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Attribute/ECStaminaSet.h"
#include "GameFramework/Pawn.h"

void UStaminaViewModel::BindCallbacksToDependencies(AActor* ContextActor)
{
	UnbindAttributeDelegates();
	Super::BindCallbacksToDependencies(ContextActor);

	UAbilitySystemComponent* ASC = ContextActor
		? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ContextActor)
		: nullptr;
	if (!ASC)
	{
		ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetPlayerPawn());
	}

	if (ASC)
	{
		const UECStaminaSet* StaminaSet = ASC->GetSet<UECStaminaSet>();
		if (StaminaSet)
		{
			AbilitySystemComponent = ASC;
			StaminaChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(
				UECStaminaSet::GetStaminaAttribute()).AddUObject(
					this,
					&ThisClass::HandleStaminaChanged);
			MaxStaminaChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(
				UECStaminaSet::GetMaxStaminaAttribute()).AddUObject(
					this,
					&ThisClass::HandleMaxStaminaChanged);
			
			SetCurrentStamina(FMath::RoundToInt(StaminaSet->GetStamina()));
			SetMaxStamina(FMath::RoundToInt(StaminaSet->GetMaxStamina()));
		}
	}
}

void UStaminaViewModel::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentStamina);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(MaxStamina);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStaminaPercent);
}

void UStaminaViewModel::BeginDestroy()
{
	UnbindAttributeDelegates();
	Super::BeginDestroy();
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
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxStamina, NewMaxStamina))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStaminaPercent);
	}
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

void UStaminaViewModel::HandleStaminaChanged(const FOnAttributeChangeData& Data)
{
	SetCurrentStamina(FMath::RoundToInt(Data.NewValue));
}

void UStaminaViewModel::HandleMaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	SetMaxStamina(FMath::RoundToInt(Data.NewValue));
}

void UStaminaViewModel::UnbindAttributeDelegates()
{
	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		if (StaminaChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(
				UECStaminaSet::GetStaminaAttribute()).Remove(StaminaChangedHandle);
		}
		if (MaxStaminaChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(
				UECStaminaSet::GetMaxStaminaAttribute()).Remove(MaxStaminaChangedHandle);
		}
	}

	StaminaChangedHandle.Reset();
	MaxStaminaChangedHandle.Reset();
	AbilitySystemComponent.Reset();
}
