// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ECCombatFeedbackDataAsset.generated.h"

class UNiagaraSystem;
class USoundBase;

/** 태그의 조합 결과로 실행할 연출 데이터 구조체 */
USTRUCT(BlueprintType)
struct FCombatFeedbackResult
{
	GENERATED_BODY()

	/** 타격 시 직접 재생할 사운드 에셋입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> HitSound = nullptr;

	/** 타격 시 실행할 히트 스탑의 지속 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitStopTimeDuration = 0.3f;

	/** 타격 시 적용할 히트 스탑의 시간 배율입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitStopTimeDilation = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UNiagaraSystem> HitVFX = nullptr;

	/**
	 * 히트 VFX 생성 위치에 더할 오프셋입니다.
	 * 부착 대상의 회전과 무관하게 월드 축 기준으로 적용합니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (Units = "cm"))
	FVector HitVFXLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector VFXScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UNiagaraSystem> PlayerGroundWaveVFX = nullptr;
};

/** 하나의 무기 안에서 공격자, 피격자, 공격 종류에 따른 피드백 정보입니다. */
USTRUCT(BlueprintType)
struct FWeaponCombatFeedbackEntry
{
	GENERATED_BODY()

	/** 공격 주체를 구분하는 태그입니다. 비워 두면 모든 공격자를 허용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Character.Type"))
	FGameplayTag SourceTag;

	/** 피격 대상을 구분하는 태그입니다. 비워 두면 모든 대상을 허용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Character.Type"))
	FGameplayTag TargetTag;

	/** 타격한 공격의 종류입니다. 비워 두면 약공격과 강공격 모두에 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Attack"))
	FGameplayTag AttackTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCombatFeedbackResult FeedbackResult;
};

/** 같은 무기 태그에서 사용할 피드백들을 한 묶음으로 관리합니다. */
USTRUCT(BlueprintType)
struct FWeaponCombatFeedbackGroup
{
	GENERATED_BODY()

	/** 이 그룹이 담당하는 무기 태그입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Weapon"))
	FGameplayTag WeaponTag;

	/** 적 종류와 약공격/강공격 조합별 피드백입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWeaponCombatFeedbackEntry> FeedbackEntries;
};

/** 일반 가드 성공 시 공통으로 사용할 피드백입니다. */
USTRUCT(BlueprintType)
struct FGuardCombatFeedbackResult
{
	GENERATED_BODY()

	/** 일반 가드 상태에서 공격을 막았을 때 재생할 공용 사운드 에셋입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> GuardSound = nullptr;
};

/** 패링 성공 지점에서 실행할 연출 데이터입니다. */
USTRUCT(BlueprintType)
struct FParryCombatFeedbackResult
{
	GENERATED_BODY()

	/** 패링 성공 시 직접 재생할 사운드 에셋입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> ParrySound = nullptr;

	/** 패링 성공 지점에 생성할 나이아가라 시스템입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> ParryVFX = nullptr;

	/** 접촉 지점에 월드 축 기준으로 더할 위치 오프셋입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (Units = "cm"))
	FVector ParryVFXLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector ParryVFXScale = FVector::OneVector;
};

/** 특정 에너미 타입을 패링했을 때 사용할 피드백입니다. */
USTRUCT(BlueprintType)
struct FParryCombatFeedbackEntry
{
	GENERATED_BODY()

	/** DT_Enemy에서 해당 에너미 클래스에 연결한 태그입니다. 비워 두면 기본값으로 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Character.Type"))
	FGameplayTag EnemyTypeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FParryCombatFeedbackResult FeedbackResult;
};

/** 전투 피드백 데이터를 관리하는 데이터 에셋입니다. */
UCLASS(BlueprintType)
class EMPTYCITY_API UECCombatFeedbackDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 무기 태그별로 분리한 전투 피드백 데이터입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Feedback")
	TArray<FWeaponCombatFeedbackGroup> WeaponFeedbackGroups;

	/** 무기 및 에너미 타입과 관계없이 사용하는 공용 가드 피드백입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Feedback")
	FGuardCombatFeedbackResult GuardFeedback;

	/** 에너미 타입별 패링 성공 피드백입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Feedback")
	TArray<FParryCombatFeedbackEntry> ParryFeedbackEntries;

	/** 공격자/피격자/무기/공격 태그의 매칭 결과를 반환합니다. */
	FCombatFeedbackResult GetFeedbackResult(
		FGameplayTag InSourceTag,
		FGameplayTag InTargetTag,
		FGameplayTag InWeaponTag,
		FGameplayTag InAttackTag) const;

	/** 에너미 타입에 가장 구체적으로 일치하는 패링 피드백을 반환합니다. */
	FParryCombatFeedbackResult GetParryFeedbackResult(FGameplayTag InEnemyTypeTag) const;
};
