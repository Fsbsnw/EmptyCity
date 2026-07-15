// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/ECTimeSubsystem.h"

#include "ECGameplayMessageSubsystem.h"
#include "ECGameplayTags.h"
#include "ECMessageTypes.h"
#include "ECRegionSubsystem.h"

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

	LastTimeRelevantRegionTag = ECGameplayTags::MapNode_Location_Home;
	
	UECRegionSubsystem& RegionSubsystem = UECRegionSubsystem::Get(this);
	RegionSubsystem.OnRegionChanged.AddUObject(this, &ThisClass::HandleRegionChanged);
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
		++ECDay;
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
	// 교환소는 시간 흐름 없음
	if (NewRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_TradingPost))
	{
		return;
	}

	const FGameplayTag OriginRegionTag = LastTimeRelevantRegionTag.IsValid() ? LastTimeRelevantRegionTag : PreviousRegionTag;

	// 다른 지역에서 집으로 돌아오면 저녁
	if (NewRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_Home))
	{
		// 집 -> 교환소 -> 집이 아닌 경우
		if (!OriginRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_Home))
		{
			AdvanceTimeOfDay();
		}
	}
	// 집에서 다른 지역으로 이동
	else if (OriginRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_Home))
	{
		AdvanceTimeOfDay();
	}

	LastTimeRelevantRegionTag = NewRegionTag;
}

void UECTimeSubsystem::SleepUntilTomorrow()
{
	++ECDay;

	SetTimeOfDay(EECTimeOfDay::Morning, true);
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