// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ECTimeSubsystem.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Time_Morning);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Time_Afternoon);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Time_Evening);

UENUM(BlueprintType)
enum class EECTimeOfDay : uint8
{
	Morning		UMETA(DisplayName = "Morning"),
	Afternoon	UMETA(DisplayName = "Afternoon"),
	Evening		UMETA(DisplayName = "Evening")
};

DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FOnGameTimeChanged,
	int32,
	EECTimeOfDay,
	EECTimeOfDay
);

/**
 * 게임의 진행 시간을 담당하는 서브시스템입니다.
 */
UCLASS()
class EMPTYCITY_API UECTimeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UECTimeSubsystem& Get(const UObject* WorldContextObject);
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 다음 날 아침으로 시간을 변경합니다. */
	UFUNCTION(BlueprintCallable)
	void SleepUntilTomorrow();

	/** 현재 시간대를 다음 시간대로 진행합니다. */
	UFUNCTION(BlueprintCallable)
	void AdvanceTimeOfDay();
	
	/** 현재 시간대를 반환합니다. */
	UFUNCTION(BlueprintPure)
	EECTimeOfDay GetTimeOfDay() const { return TimeOfDay; }

	/** 현재 날짜를 반환합니다. */
	UFUNCTION(BlueprintPure)
	int32 GetDay() const { return ECDay; }

private:
	/** 지역 변경에 따라 시간대를 진행합니다. */
	UFUNCTION()
	void HandleRegionChanged(FGameplayTag PreviousRegionTag, FGameplayTag NewRegionTag);

	/** 현재 시간대를 변경하고 알림을 전달합니다. */
	void SetTimeOfDay(EECTimeOfDay NewTimeOfDay, bool bForceNotify = false);

	/** 시간대에 대응하는 게임플레이 태그를 반환합니다. */
	FGameplayTag GetTimeTag(EECTimeOfDay Time) const;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 ECDay = 1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EECTimeOfDay TimeOfDay = EECTimeOfDay::Morning;

	/** 마지막으로 시간을 진행시킨 지역 태그입니다. */
	FGameplayTag LastTimeRelevantRegionTag;
};
