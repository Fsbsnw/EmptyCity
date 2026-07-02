// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "CutsceneDataAsset.generated.h"

class ULevelSequence;
class UMediaSource;

UENUM(BlueprintType)
enum class ECutsceneType : uint8
{
	Video,
	LevelSequence
};

/** 실행할 컷씬 데이터 */
USTRUCT(BlueprintType)
struct FCutsceneData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECutsceneType CutsceneType = ECutsceneType::LevelSequence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "CutsceneType == ECutsceneType::Video", EditConditionHides))
	TSoftObjectPtr<UMediaSource> MediaSource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "CutsceneType == ECutsceneType::LevelSequence", EditConditionHides))
	TSoftObjectPtr<ULevelSequence> LevelSequence;
};

/** 태그를 이용해서 실행할 컷씬 데이터를 관리합니다. */
USTRUCT(BlueprintType)
struct FCutsceneDataRow
{
	GENERATED_BODY()

	/** 실행할 컷씬 데이터와 매치하는 태그입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag CutsceneTag; // 예: Cutscene.Boss.Spawn

	/** 검색할 태그에 맞는 컷씬 데이터를 관리합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCutsceneData CutsceneData;
};

/**
 * 
 */
UCLASS(BlueprintType)
class EMPTYCITY_API UCutsceneDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FCutsceneDataRow> CutsceneDataRows;

	/** 태그에 맞는 컷씬 데이터를 찾아주는 함수입니다. */
	bool FindCutsceneData(FGameplayTag InTag, FCutsceneData& OutData) const;
};
