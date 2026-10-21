// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/ECCutsceneSubsystem.h"

#include "Character/Player/Controller/ECPlayerController.h"
#include "Data/Cutscene/ECCutsceneDataAsset.h"
#include "Data/Cutscene/ECCutsceneSettings.h"
#include "DefaultLevelSequenceInstanceData.h"
#include "ECGameplayTags.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaSource.h"
#include "MovieScene.h"
#include "MovieSceneObjectBindingID.h"
#include "MovieSceneSequencePlayer.h"
#include "MovieSceneSpawnable.h"
#include "Save/ECSaveSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "UI/Subsystem/UIManagerSubsystem.h"

namespace
{
	FCutscenePresentationOptions ResolvePresentationOptions(
		FGameplayTag CutsceneTag,
		const FCutsceneData& CutsceneData)
	{
		FCutscenePresentationOptions Options = CutsceneData.PresentationOptions;
		if (CutsceneData.bOverridePresentationOptions)
		{
			return Options;
		}

		// 반피 연출은 컷씬 시퀀스를 사용하지만 실제 전투는 계속 진행됩니다.
		// 기존 데이터 에셋을 다시 저장하지 않아도 플레이어가 장판을 피할 수 있게 합니다.
		if (CutsceneTag.MatchesTagExact(ECGameplayTags::Cutscene_Boss_HalfHealth))
		{
			Options.bLockPlayerInput = false;
			Options.bHideHUD = false;
			Options.bHidePlayer = false;
			Options.bHidePlayerAttachedActors = false;
			Options.bDisableCameraCuts = true;
		}

		return Options;
	}
}

void UECCutsceneSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UGameInstance* GameInstance = LocalPlayer->GetGameInstance())
		{
			if (UECSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UECSaveSubsystem>())
			{
				SaveSubsystem->RegisterSaveable(this);
			}
		}
	}
}

void UECCutsceneSubsystem::Deinitialize()
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UGameInstance* GameInstance = LocalPlayer->GetGameInstance())
		{
			if (UECSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UECSaveSubsystem>())
			{
				SaveSubsystem->UnregisterSaveable(this);
			}
		}
	}

	FinishActiveCutscene(false);
	OnCutsceneStarted.Clear();
	OnCutsceneFinished.Clear();
	CompletedCutsceneTags.Reset();
	Super::Deinitialize();
}

bool UECCutsceneSubsystem::PlayCutsceneByTag(FGameplayTag CutsceneTag, AActor* ContextActor)
{
	if (!CutsceneTag.IsValid() || bCutscenePlaying)
	{
		return false;
	}

	// 1. 프로젝트 세팅에서 글로벌 카탈로그 가져오기
	const UECCutsceneSettings* Settings = GetDefault<UECCutsceneSettings>();
	if (!Settings)
	{
		return false;
	}

	// 2. 데이터 에셋 동기 로드
	if (UECCutsceneDataAsset* CutsceneDataAsset = Settings->GlobalCutsceneDataAsset.LoadSynchronous())
	{
		FCutsceneData FoundData;
		// 3. 태그로 검색
		if (CutsceneDataAsset->FindCutsceneData(CutsceneTag, FoundData))
		{
			if (FoundData.ReplayPolicy == ECutsceneReplayPolicy::PlayOnce
				&& HasCompletedCutscene(CutsceneTag))
			{
				return false;
			}

			FoundData.PresentationOptions = ResolvePresentationOptions(CutsceneTag, FoundData);

			// 4. 찾았다면 내부 분기 실행기 호출
			if (ExecuteCutscene(CutsceneTag, FoundData, ContextActor))
			{
				return true;
			}

			return false;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("컷씬 태그를 찾을 수 없음: %s"), *CutsceneTag.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GlobalCutsceneDataAsset 로드 실패"));
	}

	return false;
}

FCutsceneData UECCutsceneSubsystem::GetCutsceneDataByTag(FGameplayTag CutsceneTag)
{
	// 1. 프로젝트 세팅에서 글로벌 카탈로그 가져오기
	const UECCutsceneSettings* Settings = GetDefault<UECCutsceneSettings>();
	if (!Settings)
	{
		return FCutsceneData();
	}

	// 2. 데이터 에셋 동기 로드
	if (UECCutsceneDataAsset* CutsceneDataAsset = Settings->GlobalCutsceneDataAsset.LoadSynchronous())
	{
		FCutsceneData FoundData;

		// 3. 태그로 검색
		if (CutsceneDataAsset->FindCutsceneData(CutsceneTag, FoundData))
		{
			FoundData.PresentationOptions = ResolvePresentationOptions(CutsceneTag, FoundData);
			return FoundData;
		}

		UE_LOG(LogTemp, Error, TEXT("컷씬 태그를 찾을 수 없음: %s"), *CutsceneTag.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GlobalCutsceneDataAsset 로드 실패"));
	}

	return FCutsceneData();
}

bool UECCutsceneSubsystem::PlayVideo(
	const FCutsceneData& CutsceneData,
	FGameplayTag CutsceneTag)
{
	const TSoftObjectPtr<UMediaSource>& VideoAsset = CutsceneData.MediaSource;
	UMediaSource* LoadedVideo = VideoAsset.LoadSynchronous();
	if (!IsValid(LoadedVideo))
	{
		UE_LOG(LogTemp, Error, TEXT("비디오 Media Source를 로드하지 못했습니다: %s"), *VideoAsset.ToString());
		return false;
	}

	const UECCutsceneSettings* Settings = GetDefault<UECCutsceneSettings>();
	if (!Settings)
	{
		return false;
	}

	UMediaPlayer* MediaPlayer = Settings->VideoMediaPlayer.LoadSynchronous();
	UClass* VideoWidgetClass = Settings->VideoWidgetClass.LoadSynchronous();
	UWorld* World = GetWorld();
	APlayerController* PlayerController = GetLocalPlayer() && World
		? GetLocalPlayer()->GetPlayerController(World)
		: nullptr;

	if (!IsValid(MediaPlayer) || !IsValid(VideoWidgetClass) || !IsValid(PlayerController))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("비디오 컷씬 설정이 올바르지 않습니다. MediaPlayer=%s, WidgetClass=%s, PlayerController=%s"),
			*GetNameSafe(MediaPlayer),
			*GetNameSafe(VideoWidgetClass),
			*GetNameSafe(PlayerController));
		return false;
	}

	UUserWidget* VideoWidget = CreateWidget<UUserWidget>(PlayerController, VideoWidgetClass);
	if (!IsValid(VideoWidget))
	{
		UE_LOG(LogTemp, Error, TEXT("비디오 컷씬 위젯을 생성하지 못했습니다: %s"), *GetNameSafe(VideoWidgetClass));
		return false;
	}

	// 레벨 블루프린트가 아니라 컷씬 시스템이 MediaPlayer의 생명주기와 종료 이벤트를 소유합니다.
	MediaPlayer->OnEndReached.RemoveAll(this);
	MediaPlayer->OnMediaOpenFailed.RemoveAll(this);
	MediaPlayer->Close();
	MediaPlayer->PlayOnOpen = true;
	MediaPlayer->SetLooping(false);

	ActiveMediaPlayer = MediaPlayer;
	ActiveVideoWidget = VideoWidget;
	ActiveCutsceneTag = CutsceneTag;
	bActiveCutscenePlaysOnce = CutsceneData.ReplayPolicy == ECutsceneReplayPolicy::PlayOnce;
	bCutscenePlaying = true;

	MediaPlayer->OnEndReached.AddDynamic(this, &ThisClass::HandleVideoPlaybackEnded);
	MediaPlayer->OnMediaOpenFailed.AddDynamic(this, &ThisClass::HandleVideoOpenFailed);

	VideoWidget->AddToViewport(Settings->VideoWidgetZOrder);

	ActiveMediaSoundComponent = NewObject<UMediaSoundComponent>(PlayerController);
	if (IsValid(ActiveMediaSoundComponent))
	{
		PlayerController->AddInstanceComponent(ActiveMediaSoundComponent);
		ActiveMediaSoundComponent->SetMediaPlayer(MediaPlayer);
		ActiveMediaSoundComponent->RegisterComponent();
	}

	if (CutsceneData.PresentationOptions.bLockPlayerInput)
	{
		InputLockedPlayerController = Cast<AECPlayerController>(PlayerController);
		if (AECPlayerController* ECPlayerController = InputLockedPlayerController.Get())
		{
			ECPlayerController->SetPlayerInputLocked(true);
		}
	}

	if (!MediaPlayer->OpenSource(LoadedVideo))
	{
		UE_LOG(LogTemp, Error, TEXT("비디오 Media Source 열기 요청에 실패했습니다: %s"), *VideoAsset.ToString());
		FinishActiveCutscene(false);
		return false;
	}

	// 게임플레이 상태를 먼저 정리한 뒤 실제 미디어 재생이 시작되도록 알립니다.
	if (CutsceneData.PresentationOptions.bHideHUD)
	{
		SetHUDHiddenForActiveCutscene(true);
	}
	OnCutsceneStarted.Broadcast(CutsceneTag);
	return true;
}

bool UECCutsceneSubsystem::PlaySequence(
	const FCutsceneData& CutsceneData,
	FGameplayTag CutsceneTag,
	AActor* ContextActor)
{
	const TSoftObjectPtr<ULevelSequence>& SequenceAsset = CutsceneData.LevelSequence;
	ULevelSequence* LoadedSequence = SequenceAsset.LoadSynchronous();
	if (!IsValid(LoadedSequence))
	{
		UE_LOG(LogTemp, Error, TEXT("Level Sequence를 로드하지 못했습니다: %s"), *SequenceAsset.ToString());
		return false;
	}

	FMovieSceneSequencePlaybackSettings PlaybackSettings;
	PlaybackSettings.bHidePlayer = CutsceneData.PresentationOptions.bHidePlayer;
	PlaybackSettings.bDisableCameraCuts = CutsceneData.PresentationOptions.bDisableCameraCuts;
	PlaybackSettings.FinishCompletionStateOverride = EMovieSceneCompletionModeOverride::ForceRestoreState;
	ALevelSequenceActor* SpawnedSequenceActor = nullptr;
	ULevelSequencePlayer* SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
		this,
		LoadedSequence,
		PlaybackSettings,
		SpawnedSequenceActor);

	if (!IsValid(SequencePlayer) || !IsValid(SpawnedSequenceActor))
	{
		UE_LOG(LogTemp, Error, TEXT("Level Sequence Player를 생성하지 못했습니다: %s"), *SequenceAsset.ToString());
		return false;
	}

	FTransform TransformOrigin = FTransform::Identity;
	bool bApplyTransformOrigin = false;

	switch (CutsceneData.TransformOriginMode)
	{
	case ECutsceneTransformOriginMode::ContextActor:
	{
		if (!IsValid(ContextActor))
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("대상 액터 기준 Transform Origin을 사용할 액터가 없습니다: %s"),
				*SequenceAsset.ToString());
			SpawnedSequenceActor->Destroy();
			return false;
		}

		// 시퀀스 제작 기준 액터의 방향을 현재 대상 액터의 방향으로 맞추는 Yaw 차이만 적용합니다.
		// 캐릭터의 Pitch/Roll 및 Scale은 카메라 트랙을 기울이거나 왜곡할 수 있으므로 제외합니다.
		const float OriginYaw = FMath::FindDeltaAngleDegrees(
			CutsceneData.AuthoredContextActorYaw,
			ContextActor->GetActorRotation().Yaw);
		TransformOrigin = FTransform(
			FRotator(0.0f, OriginYaw, 0.0f),
			ContextActor->GetActorLocation(),
			FVector::OneVector);
		bApplyTransformOrigin = true;
		break;
	}

	case ECutsceneTransformOriginMode::SpecifiedTransform:
		TransformOrigin = CutsceneData.SpecifiedTransformOrigin;
		bApplyTransformOrigin = true;
		break;

	case ECutsceneTransformOriginMode::None:
	default:
		break;
	}

	if (bApplyTransformOrigin)
	{
		UDefaultLevelSequenceInstanceData* InstanceData =
			Cast<UDefaultLevelSequenceInstanceData>(SpawnedSequenceActor->DefaultInstanceData);
		if (!IsValid(InstanceData))
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("Level Sequence Transform Origin 데이터를 가져오지 못했습니다: %s"),
				*SequenceAsset.ToString());
			SpawnedSequenceActor->Destroy();
			return false;
		}

		FVector OriginLocation = TransformOrigin.GetLocation();
		// X/Y는 선택한 기준점을 사용하지만 Z는 기준점 높이를 더하지 않고
		// 시퀀스에 작성된 Z 값에 이 오프셋만 적용합니다.
		OriginLocation.Z = CutsceneData.TransformOriginZOffset;
		TransformOrigin.SetLocation(OriginLocation);

		// 원점은 재생 시작 시점의 Transform으로 고정해 대상 액터가 움직여도 따라가지 않도록 한다.
		InstanceData->TransformOriginActor = nullptr;
		InstanceData->TransformOrigin = TransformOrigin;
		SpawnedSequenceActor->bOverrideInstanceData = true;
	}

	if (!CutsceneData.ContextActorBindingTag.IsNone())
	{
		if (!IsValid(ContextActor))
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("Level Sequence 동적 바인딩 액터가 없습니다. Sequence=%s, BindingTag=%s"),
				*SequenceAsset.ToString(),
				*CutsceneData.ContextActorBindingTag.ToString());
			SpawnedSequenceActor->Destroy();
			return false;
		}

		TArray<AActor*> BoundActors;
		BoundActors.Add(ContextActor);
		SpawnedSequenceActor->SetBindingByTag(CutsceneData.ContextActorBindingTag, BoundActors, false);

		// 시퀀스 제작용으로 배치한 동일 클래스 Spawnable도 실제 액터로 교체합니다.
		// Transform 트랙과 Animation 트랙이 서로 다른 바인딩에 있어도 한 런타임 액터에 적용되며,
		// 제작용 복제 액터는 더 이상 별도로 Spawn되지 않습니다.
		if (UMovieScene* MovieScene = LoadedSequence->GetMovieScene())
		{
			for (int32 SpawnableIndex = 0; SpawnableIndex < MovieScene->GetSpawnableCount(); ++SpawnableIndex)
			{
				const FMovieSceneSpawnable& Spawnable = MovieScene->GetSpawnable(SpawnableIndex);
				const AActor* SpawnableTemplate = Cast<AActor>(Spawnable.GetObjectTemplate());
				if (!IsValid(SpawnableTemplate) || !ContextActor->IsA(SpawnableTemplate->GetClass()))
				{
					continue;
				}

				const FMovieSceneObjectBindingID SpawnableBinding(
					UE::MovieScene::FFixedObjectBindingID(
						Spawnable.GetGuid(),
						MovieSceneSequenceID::Root));
				SpawnedSequenceActor->SetBinding(SpawnableBinding, BoundActors, false);
			}
		}
	}

	ActiveSequencePlayer = SequencePlayer;
	ActiveSequenceActor = SpawnedSequenceActor;
	ActiveCutsceneTag = CutsceneTag;
	bActiveCutscenePlaysOnce = CutsceneData.ReplayPolicy == ECutsceneReplayPolicy::PlayOnce;
	bCutscenePlaying = true;

	SequencePlayer->OnFinished.AddDynamic(this, &ThisClass::HandleSequencePlaybackEnded);

	if (AECPlayerController* PlayerController = Cast<AECPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		if (CutsceneData.PresentationOptions.bLockPlayerInput)
		{
			InputLockedPlayerController = PlayerController;
			PlayerController->SetPlayerInputLocked(true);
		}

		if (CutsceneData.PresentationOptions.bHidePlayerAttachedActors)
		{
			HidePlayerAttachedActors(PlayerController);
		}
	}

	// 보스 어빌리티/AI를 먼저 정리해야 CancelAllAbilities가
	// Sequencer가 생성할 애니메이션 몽타주를 취소하지 않습니다.
	if (CutsceneData.PresentationOptions.bHideHUD)
	{
		SetHUDHiddenForActiveCutscene(true);
	}
	OnCutsceneStarted.Broadcast(CutsceneTag);
	SequencePlayer->Play();
	return true;
}

bool UECCutsceneSubsystem::ExecuteCutscene(
	FGameplayTag CutsceneTag,
	const FCutsceneData& CutsceneData,
	AActor* ContextActor)
{
	if (CutsceneData.CutsceneType == ECutsceneType::Video)
	{
		return PlayVideo(CutsceneData, CutsceneTag);
	}

	if (CutsceneData.CutsceneType == ECutsceneType::LevelSequence)
	{
		return PlaySequence(
			CutsceneData,
			CutsceneTag,
			ContextActor);
	}

	return false;
}

void UECCutsceneSubsystem::HandleSequencePlaybackEnded()
{
	FinishActiveCutscene(true);
}

void UECCutsceneSubsystem::HandleVideoPlaybackEnded()
{
	FinishActiveCutscene(true);
}

void UECCutsceneSubsystem::HandleVideoOpenFailed(FString FailedUrl)
{
	UE_LOG(LogTemp, Error, TEXT("비디오 컷씬을 열지 못했습니다: %s"), *FailedUrl);
	FinishActiveCutscene(false);
}

void UECCutsceneSubsystem::FinishActiveCutscene(bool bMarkCompleted)
{
	if (!bCutscenePlaying
		&& !IsValid(ActiveSequencePlayer)
		&& !IsValid(ActiveSequenceActor)
		&& !IsValid(ActiveMediaPlayer)
		&& !IsValid(ActiveMediaSoundComponent)
		&& !IsValid(ActiveVideoWidget)
		&& !bHUDHiddenForActiveCutscene
		&& PlayerAttachedActorVisibilityStates.IsEmpty())
	{
		return;
	}

	const FGameplayTag FinishedCutsceneTag = ActiveCutsceneTag;

	if (bMarkCompleted && bActiveCutscenePlaysOnce && ActiveCutsceneTag.IsValid())
	{
		CompletedCutsceneTags.AddTag(ActiveCutsceneTag);
	}

	bCutscenePlaying = false;

	if (IsValid(ActiveSequencePlayer))
	{
		ActiveSequencePlayer->OnFinished.RemoveAll(this);
	}

	if (IsValid(ActiveMediaPlayer))
	{
		ActiveMediaPlayer->OnEndReached.RemoveAll(this);
		ActiveMediaPlayer->OnMediaOpenFailed.RemoveAll(this);
		ActiveMediaPlayer->Close();
	}

	if (AECPlayerController* PlayerController = InputLockedPlayerController.Get())
	{
		PlayerController->SetPlayerInputLocked(false);
	}
	InputLockedPlayerController.Reset();
	RestorePlayerAttachedActors();
	SetHUDHiddenForActiveCutscene(false);

	if (IsValid(ActiveSequenceActor))
	{
		ActiveSequenceActor->Destroy();
	}

	if (IsValid(ActiveMediaSoundComponent))
	{
		ActiveMediaSoundComponent->SetMediaPlayer(nullptr);
		ActiveMediaSoundComponent->DestroyComponent();
	}

	if (IsValid(ActiveVideoWidget))
	{
		ActiveVideoWidget->RemoveFromParent();
	}

	ActiveSequencePlayer = nullptr;
	ActiveSequenceActor = nullptr;
	ActiveMediaPlayer = nullptr;
	ActiveMediaSoundComponent = nullptr;
	ActiveVideoWidget = nullptr;
	ActiveCutsceneTag = FGameplayTag();
	bActiveCutscenePlaysOnce = false;

	if (FinishedCutsceneTag.IsValid())
	{
		OnCutsceneFinished.Broadcast(FinishedCutsceneTag, bMarkCompleted);
	}
}

void UECCutsceneSubsystem::SetHUDHiddenForActiveCutscene(bool bHidden)
{
	if (bHidden == bHUDHiddenForActiveCutscene)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	UUIManagerSubsystem* UIManager = LocalPlayer
		? LocalPlayer->GetSubsystem<UUIManagerSubsystem>()
		: nullptr;

	if (!IsValid(UIManager))
	{
		if (!bHidden)
		{
			bHUDHiddenForActiveCutscene = false;
		}
		return;
	}

	if (UIManager->SetHUDHiddenForCutscene(bHidden) || !bHidden)
	{
		bHUDHiddenForActiveCutscene = bHidden;
	}
}

void UECCutsceneSubsystem::HidePlayerAttachedActors(AECPlayerController* PlayerController)
{
	RestorePlayerAttachedActors();

	APawn* PlayerPawn = IsValid(PlayerController) ? PlayerController->GetPawn() : nullptr;
	if (!IsValid(PlayerPawn))
	{
		return;
	}

	TArray<AActor*> AttachedActors;
	PlayerPawn->GetAttachedActors(AttachedActors, true, true);

	PlayerAttachedActorVisibilityStates.Reserve(AttachedActors.Num());
	for (AActor* AttachedActor : AttachedActors)
	{
		if (!IsValid(AttachedActor))
		{
			continue;
		}

		FAttachedActorVisibilityState& VisibilityState =
			PlayerAttachedActorVisibilityStates.AddDefaulted_GetRef();
		VisibilityState.Actor = AttachedActor;
		VisibilityState.bWasHiddenInGame = AttachedActor->IsHidden();
		AttachedActor->SetActorHiddenInGame(true);
	}
}

void UECCutsceneSubsystem::RestorePlayerAttachedActors()
{
	for (const FAttachedActorVisibilityState& VisibilityState : PlayerAttachedActorVisibilityStates)
	{
		if (AActor* AttachedActor = VisibilityState.Actor.Get())
		{
			AttachedActor->SetActorHiddenInGame(VisibilityState.bWasHiddenInGame);
		}
	}

	PlayerAttachedActorVisibilityStates.Reset();
}

bool UECCutsceneSubsystem::HasCompletedCutscene(FGameplayTag CutsceneTag) const
{
	return CutsceneTag.IsValid() && CompletedCutsceneTags.HasTagExact(CutsceneTag);
}

void UECCutsceneSubsystem::ResetForDebugGameStart()
{
	FinishActiveCutscene(false);
	CompletedCutsceneTags.Reset();
}

void UECCutsceneSubsystem::WriteSaveData(FECGameplaySaveData& InOutSaveData) const
{
	InOutSaveData.CompletedCutsceneTags = CompletedCutsceneTags;
}

void UECCutsceneSubsystem::ReadSaveData(const FECGameplaySaveData& SaveData)
{
	CompletedCutsceneTags = SaveData.CompletedCutsceneTags;
}
