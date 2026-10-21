// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/ECEnemyCharacterBase.h"
#include "GameplayTagContainer.h"
#include "ECEnemyBoss.generated.h"

class UECCutsceneSubsystem;
class UAnimInstance;
class UECBossPatternComponent;

/**
 * 
 */
UCLASS()
class EMPTYCITY_API AECEnemyBoss : public AECEnemyCharacterBase
{
	GENERATED_BODY()

public:
	AECEnemyBoss(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnDeathStarted(AActor* OwningActor) override;
	virtual void OnDeathFinished(AActor* OwningActor) override;

	/** 플레이어 재도전 시 이번 전투 시도의 1회성 패턴 상태를 초기화합니다. */
	void ResetBossPatternForRetry();

private:
	UFUNCTION()
	void HandleBossHealthChanged(
		UECHealthComponent* ChangedHealthComponent,
		float OldValue,
		float NewValue,
		AActor* DamageInstigator);

	void HandleCutsceneStarted(FGameplayTag CutsceneTag);
	void HandleCutsceneFinished(FGameplayTag CutsceneTag, bool bCompleted);
	void SetBossAIForCutscenePaused(bool bPaused);
	void CacheBossAnimationState();
	void RestoreBossAnimationState();
	void HideBossForDeathCutscene();
	void RestoreBossVisibilityAfterDeathCutscene();

private:
	/** 체력 임계점 기반 장판 준비와 순차 폭발을 관리합니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Boss|Pattern", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UECBossPatternComponent> BossPatternComponent;


	/** 이 체력 비율 이하로 내려갔을 때 컷씬을 재생합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Phase Cutscene", meta=(AllowPrivateAccess="true", ClampMin="0.0", ClampMax="1.0"))
	float HalfHealthCutsceneThreshold = 0.5f;

	bool bHalfHealthCutsceneTriggered = false;
	bool bBossPhaseCutsceneInProgress = false;
	bool bBossDeathCutsceneInProgress = false;
	bool bBossAnimationStateCached = false;
	bool bBossVisibilityCachedForDeathCutscene = false;
	bool bBossWasHiddenBeforeDeathCutscene = false;

	UPROPERTY(Transient)
	TSubclassOf<UAnimInstance> CachedBossAnimInstanceClass;
};
