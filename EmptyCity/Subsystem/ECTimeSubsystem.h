// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ECTimeSubsystem.generated.h"

/**
 * 게임의 진행 시간을 담당하는 서브시스템입니다.
 */
UCLASS()
class EMPTYCITY_API UECTimeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SleepUntilTomorrow();

private:
	int32 ECDay = 1;
};
