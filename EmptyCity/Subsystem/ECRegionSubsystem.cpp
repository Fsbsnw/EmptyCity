// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/ECRegionSubsystem.h"

#include "ECGameplayMessageSubsystem.h"
#include "ECGameplayTags.h"
#include "ECMessageTypes.h"
#include "ECTimeSubsystem.h"

UECRegionSubsystem& UECRegionSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	check(World);
	UECRegionSubsystem* Router = UGameInstance::GetSubsystem<UECRegionSubsystem>(World->GetGameInstance());
	check(Router);
	return *Router;
}

void UECRegionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentRegionTag = ECGameplayTags::MapNode_Location_Home;
	PreviousRegionTag = ECGameplayTags::MapNode_Location_Home;
}

void UECRegionSubsystem::EnterRegion(FGameplayTag NewRegionTag)
{
	if (!CanEnterRegion(NewRegionTag))
	{
		return;
	}

	PreviousRegionTag = CurrentRegionTag;
	CurrentRegionTag = NewRegionTag;
	
	OnRegionChanged.Broadcast(PreviousRegionTag, CurrentRegionTag);
	
	UECGameplayMessageSubsystem& MessageSubsystem = UECGameplayMessageSubsystem::Get(this);
	FECNotificationMessage Notification;
	Notification.TargetChannel = TAG_Notification_RegionEntered;
	Notification.PayloadTag = NewRegionTag;
	MessageSubsystem.BroadcastMessage(TAG_AddNotification_Message, Notification);
}

bool UECRegionSubsystem::CanEnterRegion(FGameplayTag NewRegionTag) const
{
	if (!NewRegionTag.IsValid() || CurrentRegionTag.MatchesTagExact(NewRegionTag))
	{
		return false;
	}
	
	const bool bIsHome = NewRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_Home);
	const bool bIsTradingPost = NewRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_TradingPost);

	const UECTimeSubsystem& TimeSubsystem =	UECTimeSubsystem::Get(this);
	
	// 오후에는 집과 교환소 외의 지역으로 이동할 수 없음
	if (TimeSubsystem.GetTimeOfDay() >= EECTimeOfDay::Afternoon && !bIsHome && !bIsTradingPost)
	{
		return false;
	}

	return true;
}