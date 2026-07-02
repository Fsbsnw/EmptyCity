// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ECCutsceneSubsystem.generated.h"

class ULevelSequence;
class UMediaSource;
struct FCutsceneData;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECCutsceneSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
public:
	/** 요청한 태그에 맞는 컷씬을 실행합니다. */
	UFUNCTION(BlueprintCallable, Category = "Cutscene")
	void PlayCutsceneByTag(FGameplayTag CutsceneTag);

	/** 요청한 태그에 맞는 컷씬 데이터를 가져옵니다. */
	UFUNCTION(BlueprintCallable) // 테스트용
	FCutsceneData GetCutsceneDataByTag(FGameplayTag CutsceneTag);

private:
	/** 컷씬 데이터 타입에 맞게 실행시키는 라우터 함수입니다. */
	void ExecuteCutscene(const FCutsceneData& CutsceneData);

	/** 비디오를 실행합니다. */
	void PlayVideo(TSoftObjectPtr<UMediaSource> VideoAsset);

	/** 시네마틱 영상을 실행합니다. */
	void PlaySequence(TSoftObjectPtr<ULevelSequence> SequenceAsset);
};
