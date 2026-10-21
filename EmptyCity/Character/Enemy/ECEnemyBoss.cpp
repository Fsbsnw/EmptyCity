// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/ECEnemyBoss.h"

#include "ECGameplayTags.h"
#include "Character/Enemy/AI/ECEnemyAIController.h"
#include "Character/Enemy/Component/BossPatternComponent.h"
#include "Component/Attribute/ECHealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/ECCutsceneSubsystem.h"
#include "Subsystem/ECGameplayMessageSubsystem.h"
#include "Subsystem/ECMessageTypes.h"

namespace
{
	UECCutsceneSubsystem* GetCutsceneSubsystem(const UObject* WorldContextObject)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
		ULocalPlayer* LocalPlayer = IsValid(PlayerController) ? PlayerController->GetLocalPlayer() : nullptr;
		return LocalPlayer ? LocalPlayer->GetSubsystem<UECCutsceneSubsystem>() : nullptr;
	}
}

AECEnemyBoss::AECEnemyBoss(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BossPatternComponent = CreateDefaultSubobject<UECBossPatternComponent>(TEXT("BossPatternComponent"));
}

void AECEnemyBoss::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.RemoveDynamic(this, &ThisClass::HandleBossHealthChanged);
		HealthComponent->OnHealthChanged.AddDynamic(this, &ThisClass::HandleBossHealthChanged);
	}

	if (UECCutsceneSubsystem* CutsceneSubsystem = GetCutsceneSubsystem(this))
	{
		CutsceneSubsystem->OnCutsceneStarted.RemoveAll(this);
		CutsceneSubsystem->OnCutsceneStarted.AddUObject(this, &ThisClass::HandleCutsceneStarted);
		CutsceneSubsystem->OnCutsceneFinished.RemoveAll(this);
		CutsceneSubsystem->OnCutsceneFinished.AddUObject(this, &ThisClass::HandleCutsceneFinished);
		bHalfHealthCutsceneTriggered = CutsceneSubsystem->HasCompletedCutscene(ECGameplayTags::Cutscene_Boss_HalfHealth);
	}
}

void AECEnemyBoss::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.RemoveDynamic(this, &ThisClass::HandleBossHealthChanged);
	}

	if (UECCutsceneSubsystem* CutsceneSubsystem = GetCutsceneSubsystem(this))
	{
		CutsceneSubsystem->OnCutsceneStarted.RemoveAll(this);
		CutsceneSubsystem->OnCutsceneFinished.RemoveAll(this);
	}

	SetBossAIForCutscenePaused(false);
	Super::EndPlay(EndPlayReason);
}

void AECEnemyBoss::OnDeathStarted(AActor* OwningActor)
{
	Super::OnDeathStarted(OwningActor);

	FECBossDeathMessage Message;
	Message.BossActor = this;
	Message.CutsceneTag = ECGameplayTags::Cutscene_Boss_Death;

	// 보스는 사망 사실만 알리고, GameFlow가 사망 컷씬 재생을 결정합니다.
	UECGameplayMessageSubsystem::Get(this).BroadcastMessage(TAG_Message_Boss_Death, Message);
}

void AECEnemyBoss::OnDeathFinished(AActor* OwningActor)
{
	if (bBossDeathCutsceneInProgress)
	{
		return;
	}

	Super::OnDeathFinished(OwningActor);
}

void AECEnemyBoss::ResetBossPatternForRetry()
{
	if (BossPatternComponent)
	{
		BossPatternComponent->ResetPattern();
	}
}

void AECEnemyBoss::HandleBossHealthChanged(
	UECHealthComponent* ChangedHealthComponent,
	float OldValue,
	float NewValue,
	AActor* DamageInstigator)
{
	if (bHalfHealthCutsceneTriggered
		|| bBossPhaseCutsceneInProgress
		|| !IsValid(ChangedHealthComponent)
		|| NewValue <= 0.0f
		|| NewValue >= OldValue)
	{
		return;
	}

	const float MaxHealth = ChangedHealthComponent->GetMaxHealth();
	if (MaxHealth <= 0.0f || NewValue / MaxHealth > HalfHealthCutsceneThreshold)
	{
		return;
	}

	const float HealthRatio = NewValue / MaxHealth;

	FECBossHalfHealthReachedMessage Message;
	Message.BossActor = this;
	Message.CutsceneTag = ECGameplayTags::Cutscene_Boss_HalfHealth;
	Message.HealthRatio = HealthRatio;

	// 보스는 임계점 도달 사실만 알리고, GameFlow가 후속 진행을 결정합니다.
	UECGameplayMessageSubsystem::Get(this).BroadcastMessage(
		TAG_Message_Boss_HalfHealthReached,
		Message);
}

void AECEnemyBoss::HandleCutsceneStarted(FGameplayTag CutsceneTag)
{
	if (CutsceneTag.MatchesTagExact(ECGameplayTags::Cutscene_Boss_Death))
	{
		bBossDeathCutsceneInProgress = true;
		HideBossForDeathCutscene();
		return;
	}

	const bool bIsSpawnCutscene = CutsceneTag.MatchesTagExact(ECGameplayTags::Cutscene_Boss_Spawn);
	const bool bIsHalfHealthCutscene = CutsceneTag.MatchesTagExact(ECGameplayTags::Cutscene_Boss_HalfHealth);
	if (!bIsSpawnCutscene && !bIsHalfHealthCutscene)
	{
		return;
	}

	if (bIsHalfHealthCutscene)
	{
		bHalfHealthCutsceneTriggered = true;
	}

	CacheBossAnimationState();
	bBossPhaseCutsceneInProgress = true;
	SetBossAIForCutscenePaused(true);
}

void AECEnemyBoss::HandleCutsceneFinished(FGameplayTag CutsceneTag, bool bCompleted)
{
	if (CutsceneTag.MatchesTagExact(ECGameplayTags::Cutscene_Boss_Death))
	{
		bBossDeathCutsceneInProgress = false;
		RestoreBossVisibilityAfterDeathCutscene();
		CompleteDeathAfterExternalVisual();
		return;
	}

	const bool bIsSpawnCutscene = CutsceneTag.MatchesTagExact(ECGameplayTags::Cutscene_Boss_Spawn);
	const bool bIsHalfHealthCutscene = CutsceneTag.MatchesTagExact(ECGameplayTags::Cutscene_Boss_HalfHealth);
	if (!bBossPhaseCutsceneInProgress || (!bIsSpawnCutscene && !bIsHalfHealthCutscene))
	{
		return;
	}

	bBossPhaseCutsceneInProgress = false;
	RestoreBossAnimationState();
	SetBossAIForCutscenePaused(false);
}

void AECEnemyBoss::SetBossAIForCutscenePaused(bool bPaused)
{
	if (bPaused && ASC)
	{
		// 현재 진행 중인 공격 어빌리티가 컷씬 뒤에서도 이어지지 않도록 종료합니다.
		ASC->CancelAllAbilities();
	}

	if (AECEnemyAIController* EnemyAIController = Cast<AECEnemyAIController>(GetController()))
	{
		EnemyAIController->SetCinematicPaused(bPaused);
	}
}

void AECEnemyBoss::CacheBossAnimationState()
{
	if (bBossAnimationStateCached)
	{
		return;
	}

	if (USkeletalMeshComponent* BossMesh = GetMesh())
	{
		CachedBossAnimInstanceClass = BossMesh->GetAnimClass();
		bBossAnimationStateCached = true;
	}
}

void AECEnemyBoss::RestoreBossAnimationState()
{
	if (!bBossAnimationStateCached)
	{
		return;
	}

	if (USkeletalMeshComponent* BossMesh = GetMesh())
	{
		BossMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		if (CachedBossAnimInstanceClass)
		{
			BossMesh->SetAnimInstanceClass(CachedBossAnimInstanceClass);
		}
	}

	CachedBossAnimInstanceClass = nullptr;
	bBossAnimationStateCached = false;
}

void AECEnemyBoss::HideBossForDeathCutscene()
{
	if (bBossVisibilityCachedForDeathCutscene)
	{
		return;
	}

	bBossWasHiddenBeforeDeathCutscene = IsHidden();
	bBossVisibilityCachedForDeathCutscene = true;
	SetActorHiddenInGame(true);
}

void AECEnemyBoss::RestoreBossVisibilityAfterDeathCutscene()
{
	if (!bBossVisibilityCachedForDeathCutscene)
	{
		return;
	}

	SetActorHiddenInGame(bBossWasHiddenBeforeDeathCutscene);
	bBossWasHiddenBeforeDeathCutscene = false;
	bBossVisibilityCachedForDeathCutscene = false;
}
