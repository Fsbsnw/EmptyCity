// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/BedMenuViewModel.h"

#include "Actor/Bed/ECBed.h"

void UBedMenuViewModel::BindCallbacksToDependencies(AActor* ContextActor)
{
	TargetBed = Cast<AECBed>(ContextActor);
}

void UBedMenuViewModel::RequestSleepSequence()
{
	if (IsValid(TargetBed))
	{
		TargetBed->StartSleepSequence();
	}
}