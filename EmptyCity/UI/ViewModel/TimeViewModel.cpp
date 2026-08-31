// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/TimeViewModel.h"

#include "Subsystem/ECTimeSubsystem.h"

void UTimeViewModel::BindCallbacksToDependencies(AActor* ContextActor)
{
	UECTimeSubsystem& TimeSubsystem = UECTimeSubsystem::Get(this);
	TimeSubsystem.OnGameTimeChanged.AddUObject(this, &UTimeViewModel::HandleTimeChanged);

	HandleTimeChanged(TimeSubsystem.GetDay(), TimeSubsystem.GetTimeOfDay());
}

void UTimeViewModel::BroadcastInitialValues()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentDay);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentTimeOfDay);
}

void UTimeViewModel::SetCurrentDay(int32 NewCurrentDay)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentDay, NewCurrentDay);
}

void UTimeViewModel::SetCurrentTimeOfDay(const FText& NewCurrentTimeOfDay)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentTimeOfDay, NewCurrentTimeOfDay);
}

void UTimeViewModel::HandleTimeChanged(int32 NewDay, EECTimeOfDay NewCurrentTimeOfDay)
{
	SetCurrentDay(NewDay);
	
	switch (NewCurrentTimeOfDay)
	{
	case EECTimeOfDay::Morning:
		SetCurrentTimeOfDay(FText::FromString(TEXT("오전")));
		break;

	case EECTimeOfDay::Afternoon:
		SetCurrentTimeOfDay(FText::FromString(TEXT("오후")));
		break;

	case EECTimeOfDay::Evening:
		SetCurrentTimeOfDay(FText::FromString(TEXT("저녁")));
		break;
	}
}