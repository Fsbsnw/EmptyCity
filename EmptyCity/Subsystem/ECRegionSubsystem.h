// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Region/ECRegionTableRow.h"
#include "GameplayTagContainer.h"
#include "Save/ECSaveable.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ECRegionSubsystem.generated.h"

class AECRegionTeleportPoint;
class UDataTable;

DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnRegionChanged,
	FGameplayTag,
	FGameplayTag
);

DECLARE_MULTICAST_DELEGATE(FOnRegionVisitHistoryChanged);

/** 지역 이동 요청이 거절된 게임 규칙상의 이유입니다. */
UENUM(BlueprintType)
enum class EECRegionTravelFailureReason : uint8
{
	None,
	TravelInProgress,
	InvalidTarget,
	AlreadyInRegion,
	RegionLocked,
	BossLocked,
	DailyTravelLimit
};

DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FOnRegionTravelRejected,
	FGameplayTag,
	FGameplayTag,
	EECRegionTravelFailureReason);

/**
 * 플레이어가 위치한 맵 지역 정보를 담당하는 서브시스템입니다.
 */
UCLASS()
class EMPTYCITY_API UECRegionSubsystem : public UGameInstanceSubsystem, public IECSaveable
{
	GENERATED_BODY()
public:
	/** 광신도 본거지 보스 전투 구역이 열리는 날짜입니다. */
	static constexpr int32 CultistBossBattleUnlockDay = 9;

	static UECRegionSubsystem& Get(const UObject* WorldContextObject);
	
public:
	virtual void WriteSaveData(FECGameplaySaveData& InOutSaveData) const override;
	virtual void ReadSaveData(const FECGameplaySaveData& SaveData) override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 현재 지역을 새로운 지역으로 변경합니다. */
	UFUNCTION(BlueprintCallable)
	void EnterRegion(FGameplayTag NewRegionTag);

	/** 현재 월드에 배치된 해당 지역의 텔레포트 지점으로 이동합니다. */
	UFUNCTION(BlueprintCallable, Category="Region|Travel")
	bool RequestTravel(FGameplayTag TargetRegionTag);

	/** 세이브에 기록된 시작 지점으로 이동합니다. 일반 지역 이동 조건과 시간 진행은 적용하지 않습니다. */
	bool RestoreSavedSpawnPoint(FGameplayTag SavedSpawnPointTag);

	/** 사망 복귀 지점으로 즉시 이동합니다. 같은 지역 이동과 일반 지역 이동 조건을 허용하지 않습니다. */
	bool RespawnAtRegion(FGameplayTag TargetRegionTag);

	/** 세이브 없는 디버그 시작을 위해 현재 지역과 방문 기록을 초기 상태로 되돌립니다. */
	void ResetForDebugGameStart();

	/** 현재 상태에서 해당 지역에 진입할 수 있는지 반환합니다. */
	UFUNCTION(BlueprintCallable)
	bool CanEnterRegion(FGameplayTag NewRegionTag) const;

	/** 현재 게임 규칙에서 해당 지역 이동이 거절되는 이유를 반환합니다. */
	UFUNCTION(BlueprintPure, Category="Region|Travel")
	EECRegionTravelFailureReason GetTravelFailureReason(FGameplayTag NewRegionTag) const;

	/** 현재 지역 태그를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Region")
	FGameplayTag GetCurrentRegionTag() const { return CurrentRegionTag; }

	/** 이전 지역 태그를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Region")
	FGameplayTag GetPreviousRegionTag() const { return PreviousRegionTag; }

	/** 지역 태그와 RowName이 일치하는 지역 정의를 반환합니다. 찾지 못하면 nullptr입니다. */
	const FECRegionTableRow* FindRegionDefinition(FGameplayTag RegionTag) const;

	/** 블루프린트에서 지역 태그로 지역 정의를 조회합니다. */
	UFUNCTION(BlueprintPure, Category="Region")
	bool GetRegionDefinition(FGameplayTag RegionTag, FECRegionTableRow& OutDefinition) const;

	/** 성공한 지역 이동 기록을 시간순으로 반환합니다. */
	const TArray<FECRegionVisitRecord>& GetVisitHistory() const { return VisitHistory; }

	/** 한 번이라도 실제 이동으로 방문한 지역인지 반환합니다. */
	UFUNCTION(BlueprintPure, Category="Region|Travel")
	bool HasVisitedRegion(FGameplayTag RegionTag) const;

public:
	/** 지역이 변경될 때 호출됩니다. */
	FOnRegionChanged OnRegionChanged;

	/** 방문 기록이 추가되거나 세이브 데이터로 복원될 때 호출됩니다. */
	FOnRegionVisitHistoryChanged OnRegionVisitHistoryChanged;

	/** 지역 이동 요청이 게임 규칙에 의해 거절될 때 호출됩니다. */
	FOnRegionTravelRejected OnRegionTravelRejected;

private:
	/** 검사가 끝난 지역을 현재 지역으로 반영하고 진입 이벤트를 전송합니다. */
	void CommitRegionEntered(FGameplayTag NewRegionTag);

	/** 지정한 지역의 이동 지점을 찾습니다. */
	AECRegionTeleportPoint* FindRegionTeleportPoint(FGameplayTag TargetRegionTag) const;

	/** 일반 이동 규칙을 적용하지 않고 해당 지역의 배치 지점으로 플레이어를 이동합니다. */
	bool TeleportToRegionSpawnPoint(FGameplayTag TargetRegionTag, const TCHAR* Context);

	/** 성공한 실제 이동을 방문 기록에 추가합니다. */
	void RecordSuccessfulVisit(FGameplayTag RegionTag);

	/** 쉘터와 교환소를 제외한 기록 대상 지역인지 반환합니다. */
	bool ShouldRecordVisit(FGameplayTag RegionTag) const;

	/** 현재 지역 태그입니다. */
	FGameplayTag CurrentRegionTag;

	/** 이전 지역 태그입니다. */
	FGameplayTag PreviousRegionTag;

	/** 프로젝트 설정을 통해 로드한 지역 정의 테이블입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> RegionDefinitionTable;

	/** 성공한 실제 이동의 방문 기록입니다. */
	UPROPERTY(Transient)
	TArray<FECRegionVisitRecord> VisitHistory;

	/** 페이드 전환을 포함한 지역 이동이 진행 중인지 여부입니다. */
	bool bTravelInProgress = false;
};
