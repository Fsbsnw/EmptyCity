// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/ECTimeSubsystem.h"

void UECTimeSubsystem::SleepUntilTomorrow()
{
	UE_LOG(LogTemp, Warning, TEXT("어제 : %d일, 오늘 : %d일"), ECDay++, ECDay);
}
