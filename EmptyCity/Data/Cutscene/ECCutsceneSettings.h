// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ECCutsceneSettings.generated.h"

class UCutsceneDataAsset;
/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Cutscene Settings"))
class EMPTYCITY_API UECCutsceneSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "Cutscene Data")
	TSoftObjectPtr<UCutsceneDataAsset> GlobalCutsceneDataAsset;
};
