// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/ECRegionSubsystem.h"

#include "ECGameplayMessageSubsystem.h"
#include "ECGameplayTags.h"
#include "ECMessageTypes.h"
#include "ECTimeSubsystem.h"
#include "EngineUtils.h"
#include "Data/Region/ECRegionSettings.h"
#include "Engine/DataTable.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Actor/RegionTeleportPoint/ECRegionTeleportPoint.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "Save/ECSaveSubsystem.h"
#include "Subsystem/ECCutsceneSubsystem.h"
#include "UI/Subsystem/UIManagerSubsystem.h"

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
	Collection.InitializeDependency<UECSaveSubsystem>();

	CurrentRegionTag = ECGameplayTags::MapNode_Location_Shelter;
	PreviousRegionTag = ECGameplayTags::MapNode_Location_Shelter;
	VisitHistory.Reset();

	if (UECSaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UECSaveSubsystem>())
	{
		SaveSubsystem->RegisterSaveable(this);
	}

	const UECRegionSettings* Settings = GetDefault<UECRegionSettings>();
	if (!Settings || Settings->RegionDefinitionTable.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("Region Settings에 Region Definition Table이 설정되지 않았습니다."));
		return;
	}

	RegionDefinitionTable = Settings->RegionDefinitionTable.LoadSynchronous();
	if (!IsValid(RegionDefinitionTable))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Region Definition Table을 로드하지 못했습니다: %s"),
			*Settings->RegionDefinitionTable.ToSoftObjectPath().ToString());
		return;
	}

	if (RegionDefinitionTable->GetRowStruct() != FECRegionTableRow::StaticStruct())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Region Definition Table의 행 구조가 FECRegionTableRow가 아닙니다: %s"),
			*GetNameSafe(RegionDefinitionTable));
		RegionDefinitionTable = nullptr;
	}
}

void UECRegionSubsystem::Deinitialize()
{
	if (UECSaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UECSaveSubsystem>())
	{
		SaveSubsystem->UnregisterSaveable(this);
	}

	VisitHistory.Reset();
	RegionDefinitionTable = nullptr;
	Super::Deinitialize();
}

void UECRegionSubsystem::EnterRegion(FGameplayTag NewRegionTag)
{
	if (!CanEnterRegion(NewRegionTag))
	{
		return;
	}

	CommitRegionEntered(NewRegionTag);
}

bool UECRegionSubsystem::RequestTravel(const FGameplayTag TargetRegionTag)
{
	if (bTravelInProgress)
	{
		OnRegionTravelRejected.Broadcast(
			CurrentRegionTag,
			TargetRegionTag,
			EECRegionTravelFailureReason::TravelInProgress);
		return false;
	}

	const EECRegionTravelFailureReason FailureReason = GetTravelFailureReason(TargetRegionTag);
	if (FailureReason != EECRegionTravelFailureReason::None)
	{
		OnRegionTravelRejected.Broadcast(CurrentRegionTag, TargetRegionTag, FailureReason);
		if (FailureReason == EECRegionTravelFailureReason::DailyTravelLimit
			|| FailureReason == EECRegionTravelFailureReason::RegionLocked)
		{
			FECNotificationMessage Notification;
			Notification.TargetChannel = TAG_Notification_Warning_RegionTravelDenied;
			Notification.PayloadTag = TAG_Notification_Warning_RegionTravelDenied;
			Notification.PayloadMessage = FailureReason == EECRegionTravelFailureReason::RegionLocked
				? NSLOCTEXT(
					"Notification",
					"RegionLocked",
					"현재 잠겨 있는 지역이라 이동할 수 없습니다.")
				: NSLOCTEXT(
					"Notification",
					"RegionTravelDenied",
					"오늘은 다른 지역으로 이동할 수 없습니다.");
			if (const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
			{
				Notification.TargetPlayer = PlayerController->PlayerState;
			}
			UECGameplayMessageSubsystem::Get(this).BroadcastMessage(
				TAG_AddNotification_Message,
				Notification);
		}
		return false;
	}

	AECRegionTeleportPoint* TeleportPoint = FindRegionTeleportPoint(TargetRegionTag);
	if (!IsValid(TeleportPoint))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("지역 이동 지점을 찾을 수 없습니다: %s"),
			*TargetRegionTag.ToString());
		return false;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!IsValid(PlayerController))
	{
		UE_LOG(LogTemp, Warning, TEXT("지역 이동을 위한 PlayerController를 찾을 수 없습니다."));
		return false;
	}

	AECPlayerController* ECPlayerController = Cast<AECPlayerController>(PlayerController);
	if (!IsValid(ECPlayerController))
	{
		UE_LOG(LogTemp, Warning, TEXT("지역 이동 입력을 제한할 ECPlayerController를 찾을 수 없습니다."));
		return false;
	}

	APawn* PlayerPawn = PlayerController->GetPawn();
	if (!IsValid(PlayerPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("지역 이동을 위한 PlayerPawn을 찾을 수 없습니다."));
		return false;
	}

	UUIManagerSubsystem* UIManagerSubsystem = UUIManagerSubsystem::Get(PlayerController);
	if (!IsValid(UIManagerSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("지역 이동 페이드 연출을 위한 UIManagerSubsystem을 찾을 수 없습니다."));
		return false;
	}

	const FVector DestinationLocation = TeleportPoint->GetActorLocation();
	const FRotator DestinationRotation = TeleportPoint->GetActorRotation();
	FGameplayTag FirstVisitCutsceneTag;
	if (ShouldRecordVisit(TargetRegionTag) && !HasVisitedRegion(TargetRegionTag))
	{
		if (const FECRegionTableRow* RegionDefinition = FindRegionDefinition(TargetRegionTag))
		{
			FirstVisitCutsceneTag = RegionDefinition->FirstVisitCutsceneTag;
		}
	}

	const TWeakObjectPtr<APlayerController> WeakPlayerController = PlayerController;
	const TWeakObjectPtr<AECPlayerController> WeakECPlayerController = ECPlayerController;
	const TWeakObjectPtr<APawn> WeakPlayerPawn = PlayerPawn;

	bTravelInProgress = true;
	ECPlayerController->SetPlayerInputLocked(true);

	const bool bFadeStarted = UIManagerSubsystem->PlayFadeTransition(
		FOnFadeEvent::CreateWeakLambda(
			this,
			[this, WeakPlayerController, WeakPlayerPawn, DestinationLocation, DestinationRotation,
				TargetRegionTag, FirstVisitCutsceneTag]()
			{
				APlayerController* ValidPlayerController = WeakPlayerController.Get();
				APawn* ValidPlayerPawn = WeakPlayerPawn.Get();
				if (!IsValid(ValidPlayerController) || !IsValid(ValidPlayerPawn))
				{
					UE_LOG(LogTemp, Warning, TEXT("페이드 도중 지역 이동 대상 플레이어가 유효하지 않게 되었습니다."));
					return;
				}

				if (!ValidPlayerPawn->TeleportTo(DestinationLocation, DestinationRotation))
				{
					UE_LOG(
						LogTemp,
						Warning,
						TEXT("지역 이동에 실패했습니다: %s"),
						*TargetRegionTag.ToString());
					return;
				}

				// Pawn이 Controller Rotation을 사용하는 경우 카메라 방향도 맞춰줍니다.
				ValidPlayerController->SetControlRotation(DestinationRotation);
				CommitRegionEntered(TargetRegionTag);

				// 화면이 완전히 검은 상태에서 컷씬 카메라와 HUD 상태를 먼저 적용합니다.
				// 이 콜백이 끝난 뒤 페이드 인이 시작되므로 이동 완료 시점이 먼저 노출되지 않습니다.
				if (FirstVisitCutsceneTag.IsValid())
				{
					UECCutsceneSubsystem* CutsceneSubsystem = nullptr;
					if (ULocalPlayer* LocalPlayer = ValidPlayerController->GetLocalPlayer())
					{
						CutsceneSubsystem = LocalPlayer->GetSubsystem<UECCutsceneSubsystem>();
					}

					if (!IsValid(CutsceneSubsystem)
						|| !CutsceneSubsystem->PlayCutsceneByTag(FirstVisitCutsceneTag, ValidPlayerPawn))
					{
						UE_LOG(
							LogTemp,
							Warning,
							TEXT("지역 최초 방문 컷씬을 시작하지 못했습니다. Region=%s, Cutscene=%s"),
							*TargetRegionTag.ToString(),
							*FirstVisitCutsceneTag.ToString());
					}
				}

				RecordSuccessfulVisit(TargetRegionTag);

				UE_LOG(LogTemp, Log, TEXT("지역 이동 완료: %s"), *TargetRegionTag.ToString());
			}),
		1.f,
		FOnFadeEvent::CreateWeakLambda(this, [this, WeakECPlayerController]()
		{
			if (AECPlayerController* ValidECPlayerController = WeakECPlayerController.Get())
			{
				ValidECPlayerController->SetPlayerInputLocked(false);
			}
			bTravelInProgress = false;
		}));

	if (!bFadeStarted)
	{
		ECPlayerController->SetPlayerInputLocked(false);
		bTravelInProgress = false;
		UE_LOG(LogTemp, Warning, TEXT("지역 이동 페이드 연출을 시작하지 못했습니다."));
		return false;
	}

	return true;
}

bool UECRegionSubsystem::RestoreSavedSpawnPoint(const FGameplayTag SavedSpawnPointTag)
{
	const FGameplayTag TargetRegionTag = SavedSpawnPointTag.MatchesTagExact(
		ECGameplayTags::MapNode_Location_CultistBase_BossBattle)
			? ECGameplayTags::MapNode_Location_CultistBase_BossBattle
			: ECGameplayTags::MapNode_Location_Shelter;

	return TeleportToRegionSpawnPoint(TargetRegionTag, TEXT("세이브 시작 지점 복원"));
}

bool UECRegionSubsystem::RespawnAtRegion(const FGameplayTag TargetRegionTag)
{
	return TeleportToRegionSpawnPoint(TargetRegionTag, TEXT("사망 복귀"));
}

void UECRegionSubsystem::ResetForDebugGameStart()
{
	CurrentRegionTag = ECGameplayTags::MapNode_Location_Shelter;
	PreviousRegionTag = ECGameplayTags::MapNode_Location_Shelter;
	VisitHistory.Reset();
	bTravelInProgress = false;
	OnRegionVisitHistoryChanged.Broadcast();
}

bool UECRegionSubsystem::TeleportToRegionSpawnPoint(
	const FGameplayTag TargetRegionTag,
	const TCHAR* Context)
{
	if (!TargetRegionTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("%s 대상 지역 태그가 유효하지 않습니다."), Context);
		return false;
	}

	AECRegionTeleportPoint* TeleportPoint = FindRegionTeleportPoint(TargetRegionTag);
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	APawn* PlayerPawn = IsValid(PlayerController) ? PlayerController->GetPawn() : nullptr;
	if (!IsValid(TeleportPoint) || !IsValid(PlayerController) || !IsValid(PlayerPawn))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("%s 지점을 찾지 못했습니다: %s"),
			Context,
			*TargetRegionTag.ToString());
		return false;
	}

	const FVector DestinationLocation = TeleportPoint->GetActorLocation();
	const FRotator DestinationRotation = TeleportPoint->GetActorRotation();
	if (!PlayerPawn->TeleportTo(DestinationLocation, DestinationRotation))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("%s 텔레포트에 실패했습니다: %s"),
			Context,
			*TargetRegionTag.ToString());
		return false;
	}

	PlayerController->SetControlRotation(DestinationRotation);
	PreviousRegionTag = CurrentRegionTag;
	CurrentRegionTag = TargetRegionTag;
	bTravelInProgress = false;

	UE_LOG(LogTemp, Log, TEXT("%s 완료: %s"), Context, *TargetRegionTag.ToString());
	return true;
}

AECRegionTeleportPoint* UECRegionSubsystem::FindRegionTeleportPoint(FGameplayTag TargetRegionTag) const
{
	UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		return nullptr;
	}

	for (TActorIterator<AECRegionTeleportPoint> It(World); It; ++It)
	{
		AECRegionTeleportPoint* TravelPoint = *It;

		if (IsValid(TravelPoint) && TravelPoint->GetRegionTag() == TargetRegionTag)
		{
			return TravelPoint;
		}
	}

	return nullptr;
}

void UECRegionSubsystem::CommitRegionEntered(const FGameplayTag NewRegionTag)
{
	if (!NewRegionTag.IsValid() || CurrentRegionTag.MatchesTagExact(NewRegionTag))
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
	return GetTravelFailureReason(NewRegionTag) == EECRegionTravelFailureReason::None;
}

EECRegionTravelFailureReason UECRegionSubsystem::GetTravelFailureReason(
	const FGameplayTag NewRegionTag) const
{
	if (!NewRegionTag.IsValid())
	{
		return EECRegionTravelFailureReason::InvalidTarget;
	}

	if (CurrentRegionTag.MatchesTagExact(NewRegionTag))
	{
		return EECRegionTravelFailureReason::AlreadyInRegion;
	}

	// 채석장은 기획상 사용하지 않으며 지도에는 잠긴 지역으로 유지합니다.
	if (NewRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_Quarry))
	{
		return EECRegionTravelFailureReason::RegionLocked;
	}
	
	const bool bIsHome = NewRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_Shelter);
	const bool bIsTradingPost = NewRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_TradingPost);
	const bool bIsCultistBossBattle =
		NewRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_CultistBase_BossBattle);
	const bool bIsCultistBaseInternalTravel =
		CurrentRegionTag.MatchesTag(ECGameplayTags::MapNode_Location_CultistBase)
		&& NewRegionTag.MatchesTag(ECGameplayTags::MapNode_Location_CultistBase);

	const UECTimeSubsystem& TimeSubsystem =	UECTimeSubsystem::Get(this);

	// 보스 전투 구역은 9일차부터, 광신도 본거지에서 문을 통과할 때만 진입할 수 있습니다.
	if (bIsCultistBossBattle
		&& (TimeSubsystem.GetDay() < CultistBossBattleUnlockDay
			|| !CurrentRegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_CultistBase)))
	{
		return EECRegionTravelFailureReason::BossLocked;
	}
	
	// 오후에는 집과 교환소 외의 지역으로 이동할 수 없음
	// 단, 광신도 본거지와 그 내부 보스 전투 구역 사이의 이동은 일반 지역 이동이 아닙니다.
	if (TimeSubsystem.GetTimeOfDay() >= EECTimeOfDay::Afternoon
		&& !bIsHome
		&& !bIsTradingPost
		&& !bIsCultistBaseInternalTravel)
	{
		return EECRegionTravelFailureReason::DailyTravelLimit;
	}

	return EECRegionTravelFailureReason::None;
}

const FECRegionTableRow* UECRegionSubsystem::FindRegionDefinition(const FGameplayTag RegionTag) const
{
	if (!RegionTag.IsValid() || !IsValid(RegionDefinitionTable))
	{
		return nullptr;
	}

	return RegionDefinitionTable->FindRow<FECRegionTableRow>(RegionTag.GetTagName(), TEXT(""),false);
}

bool UECRegionSubsystem::GetRegionDefinition(const FGameplayTag RegionTag, FECRegionTableRow& OutDefinition) const
{
	OutDefinition = FECRegionTableRow();
	if (const FECRegionTableRow* Definition = FindRegionDefinition(RegionTag))
	{
		OutDefinition = *Definition;
		return true;
	}

	return false;
}

bool UECRegionSubsystem::HasVisitedRegion(const FGameplayTag RegionTag) const
{
	return VisitHistory.ContainsByPredicate([RegionTag](const FECRegionVisitRecord& Record)
	{
		return Record.RegionTag.MatchesTagExact(RegionTag);
	});
}

void UECRegionSubsystem::RecordSuccessfulVisit(const FGameplayTag RegionTag)
{
	if (!ShouldRecordVisit(RegionTag))
	{
		return;
	}

	const UECTimeSubsystem& TimeSubsystem = UECTimeSubsystem::Get(this);

	FECRegionVisitRecord& Record = VisitHistory.AddDefaulted_GetRef();
	Record.RegionTag = RegionTag;
	Record.Day = TimeSubsystem.GetDay();
	Record.TimeOfDay = TimeSubsystem.GetTimeOfDay();

	OnRegionVisitHistoryChanged.Broadcast();
}

bool UECRegionSubsystem::ShouldRecordVisit(const FGameplayTag RegionTag) const
{
	return RegionTag.IsValid()
		&& !RegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_Shelter)
		&& !RegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_TradingPost)
		&& !RegionTag.MatchesTagExact(ECGameplayTags::MapNode_Location_CultistBase_BossBattle);
}

void UECRegionSubsystem::WriteSaveData(FECGameplaySaveData& InOutSaveData) const
{
	InOutSaveData.Region.VisitHistory = VisitHistory;
	InOutSaveData.SpawnPointTag = CurrentRegionTag.MatchesTagExact(
		ECGameplayTags::MapNode_Location_CultistBase_BossBattle)
			? ECGameplayTags::MapNode_Location_CultistBase_BossBattle
			: ECGameplayTags::MapNode_Location_Shelter;
}

void UECRegionSubsystem::ReadSaveData(const FECGameplaySaveData& SaveData)
{
	VisitHistory = SaveData.Region.VisitHistory;
	VisitHistory.RemoveAll([this](const FECRegionVisitRecord& Record)
	{
		return !ShouldRecordVisit(Record.RegionTag);
	});

	OnRegionVisitHistoryChanged.Broadcast();
}
