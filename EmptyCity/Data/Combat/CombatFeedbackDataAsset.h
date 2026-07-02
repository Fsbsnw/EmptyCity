// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "CombatFeedbackDataAsset.generated.h"

class USoundBase;
class UCameraShakeBase;

/** 태그의 조합 결과로 실행할 연출 데이터 구조체 */
USTRUCT(BlueprintType)
struct FCombatFeedbackResult
{
	GENERATED_BODY()

	/** 타격 시 실행할 사운드입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> HitSound = nullptr;

	/** 타격 시 실행할 히트 스탑의 지속 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitStopTimeDuration = 0.3f;

	/** 타격 시 실행할 히트 스탑의 딜레이 시간입니다.. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitStopTimeDilation = 0.1f;
};

/** 에디터에서 설정할 조건(태그 2개)에 따른 피드백 정보 구조체 */
USTRUCT(BlueprintType)
struct FCombatFeedbackRow
{
	GENERATED_BODY()

	/** 타격한 무기의 종류입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag WeaponTag;

	/** 타격한 공격의 종류입니다.(약공격/강공격) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag AttackTag;

	/** 무기와 공격 종류에 따라 실행할 피드백 데이터입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCombatFeedbackResult FeedbackResult;
};

/** 3. 위의 행(Row)들을 배열로 들고 있을 데이터 에셋 */
UCLASS(BlueprintType)
class EMPTYCITY_API UCombatFeedbackDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 전투 피드백 데이터를 관리합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FCombatFeedbackRow> CombatFeedbackRows;

	/** 두 태그의 매칭 결과를 반환하는 함수입니다. */
	FCombatFeedbackResult GetFeedbackResult(FGameplayTag InWeaponTag, FGameplayTag InAttackTag) const;
};