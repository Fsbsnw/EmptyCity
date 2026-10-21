// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ECCutsceneSettings.generated.h"

class UECCutsceneDataAsset;
class UMediaPlayer;
class UUserWidget;
/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Cutscene Settings"))
class EMPTYCITY_API UECCutsceneSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "Cutscene Data")
	TSoftObjectPtr<UECCutsceneDataAsset> GlobalCutsceneDataAsset;

	/** 비디오 컷씬의 영상을 출력할 Media Player입니다. 연결된 Media Texture를 위젯에서 표시합니다. */
	UPROPERTY(Config, EditAnywhere, Category = "Video Cutscene")
	TSoftObjectPtr<UMediaPlayer> VideoMediaPlayer;

	/** 비디오 컷씬 동안 화면에 표시할 위젯 클래스입니다. */
	UPROPERTY(Config, EditAnywhere, Category = "Video Cutscene")
	TSoftClassPtr<UUserWidget> VideoWidgetClass;

	/** 비디오 컷씬 위젯의 뷰포트 표시 순서입니다. */
	UPROPERTY(Config, EditAnywhere, Category = "Video Cutscene", meta = (ClampMin = "0"))
	int32 VideoWidgetZOrder = 100;
};
