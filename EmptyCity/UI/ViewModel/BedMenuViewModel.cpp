// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/BedMenuViewModel.h"

#include "Actor/Bed/ECBed.h"
#include "Subsystem/ECTimeSubsystem.h"

void UBedMenuViewModel::BindCallbacksToDependencies(AActor* ContextActor)
{
	Super::BindCallbacksToDependencies(ContextActor);

	TargetBed = Cast<AECBed>(ContextActor);

	UECTimeSubsystem& TimeSubsystem = UECTimeSubsystem::Get(this);
	TimeSubsystem.OnGameTimeChanged.AddUObject(this, &ThisClass::HandleTimeChanged);
	HandleTimeChanged(TimeSubsystem.GetDay(), TimeSubsystem.GetTimeOfDay());
}

void UBedMenuViewModel::BroadcastInitialValues()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentDay);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(WakeUpDay);
}

void UBedMenuViewModel::SetCurrentDay(int32 NewCurrentDay)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentDay, NewCurrentDay);
}

void UBedMenuViewModel::SetWakeUpDay(int32 NewWakeUpDay)
{
	UE_MVVM_SET_PROPERTY_VALUE(WakeUpDay, NewWakeUpDay);
}

void UBedMenuViewModel::RequestSleepSequence()
{
	if (IsValid(TargetBed))
	{
		TargetBed->StartSleepSequence();
	}
}

void UBedMenuViewModel::RequestCloseMenu()
{
	if (IsValid(TargetBed))
	{
		TargetBed->CloseBedMenu();
	}
}

void UBedMenuViewModel::HandleTimeChanged(int32 NewDay, EECTimeOfDay NewTimeOfDay)
{
	SetCurrentDay(NewDay);
	SetWakeUpDay(FMath::Min(NewDay + 1, UECTimeSubsystem::MaxDay));
}
