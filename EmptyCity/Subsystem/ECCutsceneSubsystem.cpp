// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/ECCutsceneSubsystem.h"

#include "Data/Cutscene/CutsceneDataAsset.h"
#include "Data/Cutscene/ECCutsceneSettings.h"

void UECCutsceneSubsystem::PlayCutsceneByTag(FGameplayTag CutsceneTag)
{
	// 1. 프로젝트 세팅에서 글로벌 카탈로그 가져오기
	const UECCutsceneSettings* Settings = GetDefault<UECCutsceneSettings>();
	if (!Settings) return;

	// 2. 데이터 에셋 동기 로드
	if (UCutsceneDataAsset* CutsceneDataAsset = Settings->GlobalCutsceneDataAsset.LoadSynchronous())
	{
		FCutsceneData FoundData;
		// 3. 태그로 검색
		if (CutsceneDataAsset->FindCutsceneData(CutsceneTag, FoundData))
		{
			// 4. 찾았다면 내부 분기 실행기 호출!
			ExecuteCutscene(FoundData);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("컷씬 태그를 찾을 수 없음: %s"), *CutsceneTag.ToString());
		}
	}
}

FCutsceneData UECCutsceneSubsystem::GetCutsceneDataByTag(FGameplayTag CutsceneTag)
{
	// 1. 프로젝트 세팅에서 글로벌 카탈로그 가져오기
	const UECCutsceneSettings* Settings = GetDefault<UECCutsceneSettings>();
	if (!Settings)
	{
		return FCutsceneData();
	}

	// 2. 데이터 에셋 동기 로드
	if (UCutsceneDataAsset* CutsceneDataAsset = Settings->GlobalCutsceneDataAsset.LoadSynchronous())
	{
		FCutsceneData FoundData;

		// 3. 태그로 검색
		if (CutsceneDataAsset->FindCutsceneData(CutsceneTag, FoundData))
		{
			return FoundData;
		}

		UE_LOG(LogTemp, Error, TEXT("컷씬 태그를 찾을 수 없음: %s"), *CutsceneTag.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GlobalCutsceneDataAsset 로드 실패"));
	}

	return FCutsceneData();
}

void UECCutsceneSubsystem::PlayVideo(TSoftObjectPtr<UMediaSource> VideoAsset)
{
	UE_LOG(LogTemp, Error, TEXT("비디오 재생"));
}

void UECCutsceneSubsystem::PlaySequence(TSoftObjectPtr<ULevelSequence> SequenceAsset)
{
	UE_LOG(LogTemp, Error, TEXT("시네마틱 재생"));
}

void UECCutsceneSubsystem::ExecuteCutscene(const FCutsceneData& CutsceneData)
{
	if (CutsceneData.CutsceneType == ECutsceneType::Video)
	{
		PlayVideo(CutsceneData.MediaSource);
	}
	else if (CutsceneData.CutsceneType == ECutsceneType::LevelSequence)
	{
		PlaySequence(CutsceneData.LevelSequence);
	}
}
