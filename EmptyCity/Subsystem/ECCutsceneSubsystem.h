// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Save/ECSaveable.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ECCutsceneSubsystem.generated.h"

class ULevelSequence;
class ULevelSequencePlayer;
class UMediaPlayer;
class UMediaSource;
class UMediaSoundComponent;
class UUserWidget;
class AActor;
class ALevelSequenceActor;
class AECPlayerController;
struct FCutsceneData;

/** 컷씬 재생 상태가 정리된 뒤 호출됩니다. bool 값은 정상 종료 또는 스킵 완료 여부입니다. */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCutsceneFinished, FGameplayTag, bool);
/** 컷씬 재생 요청이 승인되어 실제 재생을 시작할 때 호출됩니다. */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCutsceneStarted, FGameplayTag);

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECCutsceneSubsystem : public ULocalPlayerSubsystem, public IECSaveable
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void WriteSaveData(FECGameplaySaveData& InOutSaveData) const override;
	virtual void ReadSaveData(const FECGameplaySaveData& SaveData) override;

	/** 요청한 태그에 맞는 컷씬을 실행합니다. */
	UFUNCTION(BlueprintCallable, Category = "Cutscene")
	bool PlayCutsceneByTag(FGameplayTag CutsceneTag, AActor* ContextActor = nullptr);

	/** 현재 컷씬이 재생 중인지 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Cutscene")
	bool IsCutscenePlaying() const { return bCutscenePlaying; }

	/** 한 번만 재생하는 컷씬이 이미 완료되었는지 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Cutscene")
	bool HasCompletedCutscene(FGameplayTag CutsceneTag) const;

	/** 세이브 없는 디버그 시작을 위해 활성 재생 상태와 완료 기록을 모두 초기화합니다. */
	void ResetForDebugGameStart();

	/** 컷씬이 종료되고 입력 및 재생 자원이 정리된 뒤 호출됩니다. */
	FOnCutsceneFinished OnCutsceneFinished;
	/** 컷씬 재생이 실제로 시작될 때 호출됩니다. */
	FOnCutsceneStarted OnCutsceneStarted;

	/** 요청한 태그에 맞는 컷씬 데이터를 가져옵니다. */
	UFUNCTION(BlueprintCallable) // 테스트용
	FCutsceneData GetCutsceneDataByTag(FGameplayTag CutsceneTag);

private:
	/** 컷씬 데이터 타입에 맞게 실행시키는 라우터 함수입니다. */
	bool ExecuteCutscene(
		FGameplayTag CutsceneTag,
		const FCutsceneData& CutsceneData,
		AActor* ContextActor);

	/** 비디오를 실행합니다. */
	bool PlayVideo(
		const FCutsceneData& CutsceneData,
		FGameplayTag CutsceneTag);

	/** 시네마틱 영상을 실행합니다. */
	bool PlaySequence(
		const FCutsceneData& CutsceneData,
		FGameplayTag CutsceneTag,
		AActor* ContextActor);

	/** Level Sequence가 정상 종료되었을 때 재생 상태와 입력 잠금을 복구합니다. */
	UFUNCTION()
	void HandleSequencePlaybackEnded();

	/** 비디오가 끝까지 재생되었을 때 컷씬을 완료 처리합니다. */
	UFUNCTION()
	void HandleVideoPlaybackEnded();

	/** 비디오를 열지 못했을 때 완료 기록 없이 재생 상태를 정리합니다. */
	UFUNCTION()
	void HandleVideoOpenFailed(FString FailedUrl);

	/** 현재 컷씬 재생 상태를 정리합니다. */
	void FinishActiveCutscene(bool bMarkCompleted);

	/** 플레이어 Pawn과 별도 Actor로 생성된 부착 장비를 숨깁니다. */
	void HidePlayerAttachedActors(AECPlayerController* PlayerController);

	/** 컷씬 시작 전에 저장한 부착 Actor의 숨김 상태를 복구합니다. */
	void RestorePlayerAttachedActors();

	/** 활성 컷씬이 소유한 HUD 숨김 상태를 변경합니다. */
	void SetHUDHiddenForActiveCutscene(bool bHidden);

private:
	struct FAttachedActorVisibilityState
	{
		TWeakObjectPtr<AActor> Actor;
		bool bWasHiddenInGame = false;
	};

	UPROPERTY(Transient)
	TObjectPtr<ULevelSequencePlayer> ActiveSequencePlayer;

	UPROPERTY(Transient)
	TObjectPtr<ALevelSequenceActor> ActiveSequenceActor;

	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> ActiveMediaPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UMediaSoundComponent> ActiveMediaSoundComponent;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> ActiveVideoWidget;

	TWeakObjectPtr<AECPlayerController> InputLockedPlayerController;

	TArray<FAttachedActorVisibilityState> PlayerAttachedActorVisibilityStates;

	FGameplayTag ActiveCutsceneTag;
	FGameplayTagContainer CompletedCutsceneTags;

	bool bCutscenePlaying = false;
	bool bActiveCutscenePlaysOnce = false;
	bool bHUDHiddenForActiveCutscene = false;
};
