// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/ECTimeSubsystem.h"

#include "ECGameplayMessageSubsystem.h"
#include "ECGameplayTags.h"
#include "ECMessageTypes.h"
#include "ECProgressionSubsystem.h"
#include "ECRegionSubsystem.h"
#include "Save/ECSaveSubsystem.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Time_Morning, "TAG.Time.Morning");
UE_DEFINE_GAMEPLAY_TAG(TAG_Time_Afternoon, "TAG.Time.Afternoon");
UE_DEFINE_GAMEPLAY_TAG(TAG_Time_Evening, "TAG.Time.Evening");

UECTimeSubsystem& UECTimeSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	check(World);
	UECTimeSubsystem* Router = UGameInstance::GetSubsystem<UECTimeSubsystem>(World->GetGameInstance());
	check(Router);
	return *Router;
}

void UECTimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UECRegionSubsystem>();
	Collection.InitializeDependency<UECSaveSubsystem>();

	LastTimeRelevantRegionTag = ECGameplayTags::MapNode_Location_Shelter;
	
	UECRegionSubsystem& RegionSubsystem = UECRegionSubsystem::Get(this);
	RegionSubsystem.OnRegionChanged.AddUObject(this, &ThisClass::HandleRegionChanged);
	
	if (UECSaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UECSaveSubsystem>())
	{
		SaveSubsystem->RegisterSaveable(this);
	}
}

void UECTimeSubsystem::Deinitialize()
{
	if (UECSaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UECSaveSubsystem>())
	{
		SaveSubsystem->UnregisterSaveable(this);
	}
	
	Super::Deinitialize();
}

void UECTimeSubsystem::AdvanceTimeOfDay()
{
	switch (TimeOfDay)
	{
	case EECTimeOfDay::Morning:
		SetTimeOfDay(EECTimeOfDay::Afternoon);
		break;

	case EECTimeOfDay::Afternoon:
		SetTimeOfDay(EECTimeOfDay::Evening);
		break;

	case EECTimeOfDay::Evening:
		ECDay = FMath::Min(ECDay + 1, MaxDay);
		SetTimeOfDay(EECTimeOfDay::Morning);
		break;

	default:
		break;
	}
}

void UECTimeSubsystem::HandleRegionChanged(FGameplayTag PreviousRegionTag, FGameplayTag NewRegionTag)
{
	if (PreviousRegionTag == NewRegionTag)
	{
		return;
	}

	// 광신도 본거지와 내부 보스 전투 구역 사이의 이동은 시간을 진행시키지 않습니다.
	if (PreviousRegionTag.MatchesTag(ECGameplayTags::MapNode_Location_CultistBase)
		&& NewRegionTag.MatchesTag(ECGameplayTags::MapNode_Location_CultistBase))
	{
		return;
	}

	// 교환소는 시간 흐름 없음
	if (NewRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_TradingPost))
	{
		return;
	}

	const FGameplayTag OriginRegionTag = LastTimeRelevantRegionTag.IsValid() ? LastTimeRelevantRegionTag : PreviousRegionTag;

	// 다른 지역에서 집으로 돌아오면 저녁
	if (NewRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_Shelter))
	{
		// 집 -> 교환소 -> 집이 아닌 경우
		if (!OriginRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_Shelter))
		{
			AdvanceTimeOfDay();
		}
	}
	// 집에서 다른 지역으로 이동
	else if (OriginRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_Shelter))
	{
		AdvanceTimeOfDay();
	}

	LastTimeRelevantRegionTag = NewRegionTag;
}

void UECTimeSubsystem::SleepUntilTomorrow()
{
	ECDay = FMath::Min(ECDay + 1, MaxDay);
	// 수면 및 일반 지역 사망 복귀는 쉘터에서 다음 이동을 시작하는 흐름입니다.
	// 이전 파밍 지역이 남아 있으면 쉘터에서 다시 나갈 때 시간이 진행되지 않으므로 기준 지역도 맞춥니다.
	LastTimeRelevantRegionTag = ECGameplayTags::MapNode_Location_Shelter;

	SetTimeOfDay(EECTimeOfDay::Morning, true);
}

void UECTimeSubsystem::ApplyTransientDebugTime(
	const int32 NewDay,
	const EECTimeOfDay NewTimeOfDay)
{
	TGuardValue<bool> DebugTimeGuard(bApplyingTransientDebugTime, true);

	ECDay = FMath::Clamp(NewDay, MinDay, MaxDay);
	LastTimeRelevantRegionTag = ECGameplayTags::MapNode_Location_Shelter;
	bCultistBossBattleUnlockNotificationSent =
		ECDay >= UECRegionSubsystem::CultistBossBattleUnlockDay;
	SetTimeOfDay(NewTimeOfDay, true);
}

void UECTimeSubsystem::SetTimeOfDay(EECTimeOfDay NewTimeOfDay, bool bForceNotify)
{
	if (TimeOfDay == NewTimeOfDay && !bForceNotify)
	{
		return;
	}
	
	TimeOfDay = NewTimeOfDay;

	// 알림 전달
	FGameplayTag TimeTag = GetTimeTag(NewTimeOfDay);
	
	FECNotificationMessage Message;
	Message.TargetChannel = TAG_Notification_TimeChanged;
	Message.PayloadTag = TimeTag;
	Message.PayloadValue = ECDay;

	UECGameplayMessageSubsystem& MessageSubsystem = UECGameplayMessageSubsystem::Get(this);
	MessageSubsystem.BroadcastMessage(TAG_AddNotification_Message, Message);

	TryBroadcastCultistBossBattleUnlockNotification();

	OnGameTimeChanged.Broadcast(ECDay, TimeOfDay);
}

void UECTimeSubsystem::TryBroadcastCultistBossBattleUnlockNotification()
{
	if (bCultistBossBattleUnlockNotificationSent
		|| ECDay < UECRegionSubsystem::CultistBossBattleUnlockDay)
	{
		return;
	}

	bCultistBossBattleUnlockNotificationSent = true;

	UECProgressionSubsystem* ProgressionSubsystem = GetGameInstance()->GetSubsystem<UECProgressionSubsystem>();
	if (ProgressionSubsystem)
	{
		ProgressionSubsystem->UnlockContent(ECGameplayTags::MapNode_Location_CultistBase_BossBattle);
	}	
}

FGameplayTag UECTimeSubsystem::GetTimeTag(EECTimeOfDay Time) const
{
	switch (Time)
	{
		case EECTimeOfDay::Morning:
			return TAG_Time_Morning;
		case EECTimeOfDay::Afternoon:
			return TAG_Time_Afternoon;
		case EECTimeOfDay::Evening:
			return TAG_Time_Evening;
	}
	return FGameplayTag();
}

void UECTimeSubsystem::WriteSaveData(FECGameplaySaveData& InOutSaveData) const
{
	InOutSaveData.Time.Day = ECDay;
	InOutSaveData.Time.TimeOfDay = TimeOfDay;
}

void UECTimeSubsystem::ReadSaveData(const FECGameplaySaveData& SaveData)
{
	ECDay = SaveData.Time.Day;
	LastTimeRelevantRegionTag = SaveData.SpawnPointTag.MatchesTagExact(
		ECGameplayTags::MapNode_Location_CultistBase_BossBattle)
			? ECGameplayTags::MapNode_Location_CultistBase_BossBattle
			: ECGameplayTags::MapNode_Location_Shelter;
	// 세이브를 불러온 것은 날짜에 새로 도달한 것이 아니므로 해금 알림을 다시 띄우지 않습니다.
	bCultistBossBattleUnlockNotificationSent = ECDay >= UECRegionSubsystem::CultistBossBattleUnlockDay;
	SetTimeOfDay(SaveData.Time.TimeOfDay, true);
}
