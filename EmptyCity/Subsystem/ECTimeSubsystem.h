// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECTimeTypes.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "Save/ECSaveable.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ECTimeSubsystem.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Time_Morning);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Time_Afternoon);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Time_Evening);

DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnGameTimeChanged,
	int32, 
	EECTimeOfDay
);

/**
 * 게임의 진행 시간을 담당하는 서브시스템입니다.
 */
UCLASS()
class EMPTYCITY_API UECTimeSubsystem : public UGameInstanceSubsystem, public IECSaveable
{
	GENERATED_BODY()
	
public:
	virtual void WriteSaveData(FECGameplaySaveData& InOutSaveData) const override;
	virtual void ReadSaveData(const FECGameplaySaveData& SaveData) override;

public:
	/** 게임에서 사용할 날짜 범위입니다. */
	static constexpr int32 MinDay = 1;
	static constexpr int32 MaxDay = 10;

	static UECTimeSubsystem& Get(const UObject* WorldContextObject);
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

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

	/** 게임에서 진행 가능한 마지막 날짜를 반환합니다. */
	UFUNCTION(BlueprintPure)
	int32 GetMaxDay() const { return MaxDay; }

	/**
	 * 세이브 파일을 만들지 않는 디버그 시작 시 날짜와 시간대를 즉시 덮어씁니다.
	 * 변경 이벤트는 전달하지만 아침 체크포인트 저장은 억제합니다.
	 */
	void ApplyTransientDebugTime(int32 NewDay, EECTimeOfDay NewTimeOfDay);

	bool IsApplyingTransientDebugTime() const { return bApplyingTransientDebugTime; }

	FOnGameTimeChanged OnGameTimeChanged;

private:
	/** 지역 변경에 따라 시간대를 진행합니다. */
	UFUNCTION()
	void HandleRegionChanged(FGameplayTag PreviousRegionTag, FGameplayTag NewRegionTag);

	/** 현재 시간대를 변경하고 알림을 전달합니다. */
	void SetTimeOfDay(EECTimeOfDay NewTimeOfDay, bool bForceNotify = false);

	/** 광신도 본거지 보스 전투 구역 해금 날짜에 도달하면 해금 알림을 한 번 전달합니다. */
	void TryBroadcastCultistBossBattleUnlockNotification();

	/** 시간대에 대응하는 게임플레이 태그를 반환합니다. */
	FGameplayTag GetTimeTag(EECTimeOfDay Time) const;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 ECDay = 1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EECTimeOfDay TimeOfDay = EECTimeOfDay::Morning;

	/** 마지막으로 시간을 진행시킨 지역 태그입니다. */
	FGameplayTag LastTimeRelevantRegionTag;

	/** 현재 런타임에서 보스 전투 구역 해금 알림을 이미 전달했는지 여부입니다. */
	bool bCultistBossBattleUnlockNotificationSent = false;

	/** 디버그 시간 덮어쓰기 이벤트가 체크포인트 저장으로 이어지지 않게 구분합니다. */
	bool bApplyingTransientDebugTime = false;
};
