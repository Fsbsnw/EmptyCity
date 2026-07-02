// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "ECProgressionSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Progression Settings"))
class EMPTYCITY_API UECProgressionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 게임 시작 시 기본으로 해금되어 있는 태그 목록 */
	UPROPERTY(EditAnywhere, config, Category = "Defaults")
	FGameplayTagContainer DefaultUnlockedTags;
};
