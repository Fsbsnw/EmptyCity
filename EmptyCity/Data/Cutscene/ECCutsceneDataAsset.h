// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ECCutsceneDataAsset.generated.h"

class ULevelSequence;
class UMediaSource;

UENUM(BlueprintType)
enum class ECutsceneType : uint8
{
	Video,
	LevelSequence
};

/** 컷씬을 다시 재생할 수 있는지 결정하는 정책입니다. */
UENUM(BlueprintType)
enum class ECutsceneReplayPolicy : uint8
{
	Repeatable UMETA(DisplayName="계속 재생"),
	PlayOnce UMETA(DisplayName="한 번만 재생")
};

/** Level Sequence의 Transform Origin을 결정하는 방식입니다. */
UENUM(BlueprintType)
enum class ECutsceneTransformOriginMode : uint8
{
	None UMETA(DisplayName="사용 안 함"),
	ContextActor UMETA(DisplayName="대상 액터 기준"),
	SpecifiedTransform UMETA(DisplayName="특정 좌표 기준")
};

/** 컷씬 동안 게임플레이 화면과 입력을 어떻게 처리할지 결정합니다. */
USTRUCT(BlueprintType)
struct FCutscenePresentationOptions
{
	GENERATED_BODY()

	/** 컷씬 동안 이동, 시점 및 어빌리티 입력을 모두 잠급니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutscene")
	bool bLockPlayerInput = true;

	/** 컷씬 동안 HUD 레이어를 숨깁니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutscene")
	bool bHideHUD = true;

	/** Level Sequence 재생 중 플레이어 Pawn을 숨깁니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutscene")
	bool bHidePlayer = true;

	/** Level Sequence 재생 중 플레이어에게 부착된 무기 및 장비 Actor를 숨깁니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutscene")
	bool bHidePlayerAttachedActors = true;

	/** Level Sequence의 Camera Cut을 무시하고 현재 플레이어 카메라를 유지합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutscene")
	bool bDisableCameraCuts = false;
};

/** 실행할 컷씬 데이터 */
USTRUCT(BlueprintType)
struct FCutsceneData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECutsceneType CutsceneType = ECutsceneType::LevelSequence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECutsceneReplayPolicy ReplayPolicy = ECutsceneReplayPolicy::Repeatable;

	/** 기본 연출 정책 대신 아래 세부 옵션을 직접 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation", meta = (InlineEditConditionToggle))
	bool bOverridePresentationOptions = false;

	/** 입력, HUD, 플레이어/장비 표시와 Camera Cut을 컷씬별로 설정합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation", meta = (EditCondition = "bOverridePresentationOptions", ShowOnlyInnerProperties))
	FCutscenePresentationOptions PresentationOptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "CutsceneType == ECutsceneType::Video", EditConditionHides))
	TSoftObjectPtr<UMediaSource> MediaSource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "CutsceneType == ECutsceneType::LevelSequence", EditConditionHides))
	TSoftObjectPtr<ULevelSequence> LevelSequence;

	/** 실행 시 전달받은 액터를 연결할 Level Sequence 바인딩 태그입니다. 비어 있으면 동적 바인딩을 사용하지 않습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "CutsceneType == ECutsceneType::LevelSequence", EditConditionHides))
	FName ContextActorBindingTag = NAME_None;

	/** Level Sequence가 사용할 원점 방식입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "CutsceneType == ECutsceneType::LevelSequence", EditConditionHides))
	ECutsceneTransformOriginMode TransformOriginMode = ECutsceneTransformOriginMode::None;

	/** 특정 좌표 기준일 때 사용할 월드 Transform입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "CutsceneType == ECutsceneType::LevelSequence && TransformOriginMode == ECutsceneTransformOriginMode::SpecifiedTransform", EditConditionHides))
	FTransform SpecifiedTransformOrigin = FTransform::Identity;

	/** 대상 액터 기준일 때 시퀀스 제작에 사용한 기준 액터의 Yaw입니다. 런타임에는 현재 액터와 이 값의 차이만 적용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "CutsceneType == ECutsceneType::LevelSequence && TransformOriginMode == ECutsceneTransformOriginMode::ContextActor", EditConditionHides, Units = "deg"))
	float AuthoredContextActorYaw = -90.0f;

	/** 기준점의 Z 대신 사용하며, 시퀀스에 작성된 Z 값에만 더해지는 높이 오프셋입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "CutsceneType == ECutsceneType::LevelSequence && TransformOriginMode != ECutsceneTransformOriginMode::None", EditConditionHides, Units = "cm"))
	float TransformOriginZOffset = 0.0f;
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

/** 태그별 컷씬 데이터를 관리하는 데이터 에셋입니다. */
UCLASS(BlueprintType)
class EMPTYCITY_API UECCutsceneDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FCutsceneDataRow> CutsceneDataRows;

	/** 태그에 맞는 컷씬 데이터를 찾아주는 함수입니다. */
	bool FindCutsceneData(FGameplayTag InTag, FCutsceneData& OutData) const;
};
