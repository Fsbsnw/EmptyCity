// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/AI/ECEnemyAIController.h"

#include "Animation/AnimInstance.h"
#include "ECGameplayTags.h"
#include "Character/Enemy/ECEnemyCharacterBase.h"
#include "Character/Player/ECPlayer.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "StateTree.h"
#include "TimerManager.h"

void UECEnemyStateTreeAIComponent::SetStateTreeAsset(UStateTree* NewStateTree)
{
	if (IsRunning())
	{
		StopLogic(TEXT("Changing enemy StateTree asset"));
	}

	StateTreeRef.SetStateTree(NewStateTree);
}

AECEnemyAIController::AECEnemyAIController()
{
	// Enemy Team
	SetGenericTeamId(FGenericTeamId(1));

	// AI Perception
	{
		AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AI Perception Component"));
		SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

		// 감지할 대상 타입을 설정합니다.
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

		AIPerceptionComponent->ConfigureSense(*SightConfig);
	}

	StateTreeComponent = CreateDefaultSubobject<UECEnemyStateTreeAIComponent>(TEXT("StateTreeComponent"));
	StateTreeComponent->SetStartLogicAutomatically(false);

	BrainComponent = StateTreeComponent;
}

void AECEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	bIsPawnDead = false;
	bPausedByDistance = false;
	bPausedForCinematic = false;

	if (!IsValid(StateTreeComponent))
	{
		return;
	}

	// Blueprint에 저장되어 있던 Controller 소유 StateTree 설정은 사용하지 않습니다.
	// 빙의한 Enemy Character 클래스의 StateTreeAsset만 런타임에 적용합니다.
	StateTreeComponent->SetStartLogicAutomatically(false);
	StateTreeComponent->SetStateTreeAsset(nullptr);

	const AECEnemyCharacterBase* EnemyCharacter = Cast<AECEnemyCharacterBase>(InPawn);
	if (!IsValid(EnemyCharacter))
	{
		UE_LOG(LogTemp, Error,
			TEXT("%s: AECEnemyCharacterBase가 아닌 Pawn을 빙의하여 StateTree를 시작할 수 없습니다."),
			*GetNameSafe(this));
		return;
	}

	UStateTree* StateTreeAsset = EnemyCharacter->GetStateTreeAsset();
	if (!IsValid(StateTreeAsset))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("%s: %s 클래스에 StateTreeAsset이 지정되지 않았습니다."),
			*GetNameSafe(this),
			*GetNameSafe(EnemyCharacter->GetClass()));
		return;
	}

	StateTreeComponent->SetStateTreeAsset(StateTreeAsset);
	StateTreeComponent->StartLogic();
	RefreshLogicPauseState();

	UE_LOG(LogTemp, Log,
		TEXT("%s: %s에 지정된 StateTree %s를 시작했습니다."),
		*GetNameSafe(this),
		*GetNameSafe(EnemyCharacter->GetClass()),
		*GetNameSafe(StateTreeAsset));
}

void AECEnemyAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(LostSightTimerHandle);
	bLostSightDeferredByActionMontage = false;

	if (IsValid(StateTreeComponent) && StateTreeComponent->IsRunning())
	{
		StateTreeComponent->StopLogic(TEXT("Enemy pawn unpossessed"));
	}

	bPausedByDistance = false;
	bPausedForCinematic = false;

	Super::OnUnPossess();
}

void AECEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	
	// AI LOD 업데이트 설정
	{
		// AI LOD 업데이트를 위한 Player를 캐싱합니다.
		CachedPlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		
		// 0.1초 ~ 1.0초 사이의 랜덤한 시간 뒤에 첫 타이머를 시작합니다.(다른 액터와 겹치지 않게)
		float InitialDelay = FMath::RandRange(0.1f, 1.0f);

		// 플레이어와의 거리에 따라 주기적으로 최적화를 결정합니다.
		GetWorldTimerManager().SetTimer(LODTimerHandle, this, &ThisClass::UpdateAILOD, 2.0f, true, InitialDelay);
	}

	// Sight Sense에 Target이 감지될 때 실행할 함수를 바인딩합니다.
	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::OnTargetDetected);
	}
}

void AECEnemyAIController::HandlePawnDeath()
{
	bIsPawnDead = true;
	bLostSightDeferredByActionMontage = false;

	GetWorldTimerManager().ClearTimer(LODTimerHandle);
	GetWorldTimerManager().ClearTimer(LostSightTimerHandle);

	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(),false);
		AIPerceptionComponent->ForgetAll();
	}

	if (StateTreeComponent)
	{
		StateTreeComponent->StopLogic(TEXT("Enemy died"));
	}

	StopMovement();
	ClearFocus(EAIFocusPriority::Gameplay);
	ClearCombatTarget();
}

void AECEnemyAIController::EngageTarget(AActor* NewTarget)
{
	AECPlayer* TargetPlayer = Cast<AECPlayer>(NewTarget);
	if (bIsPawnDead || bPausedForCinematic || !IsValid(TargetPlayer))
	{
		return;
	}

	const bool bShouldSendEngageEvent = !bIsInCombat || TargetActor != TargetPlayer;

	GetWorldTimerManager().ClearTimer(LostSightTimerHandle);
	bLostSightDeferredByActionMontage = false;

	TargetActor = TargetPlayer;
	bHasLineOfSight = true;
	bIsInCombat = true;
	LastKnownTargetLocation = TargetPlayer->GetActorLocation();

	// 거리 LOD로 잠들어 있더라도 피격됐다면 즉시 전투 로직을 깨웁니다.
	if (bPausedByDistance)
	{
		bPausedByDistance = false;
		RefreshLogicPauseState();
	}

	if (bShouldSendEngageEvent && IsValid(StateTreeComponent))
	{
		if (!StateTreeComponent->IsRunning())
		{
			StateTreeComponent->StartLogic();
		}

		if (StateTreeComponent->IsRunning() && !StateTreeComponent->IsPaused())
		{
			FStateTreeEvent Event;
			Event.Tag = ECGameplayTags::StateTree_Event_Engage;
			StateTreeComponent->SendStateTreeEvent(Event);
		}
	}
}

void AECEnemyAIController::SetCinematicPaused(bool bPaused)
{
	if (bIsPawnDead || bPausedForCinematic == bPaused)
	{
		return;
	}

	bPausedForCinematic = bPaused;

	if (bPaused)
	{
		StopMovement();
		ClearFocus(EAIFocusPriority::Gameplay);
	}

	RefreshLogicPauseState();
}

void AECEnemyAIController::UpdateAILOD()
{
	if (bIsPawnDead)
	{
		return;
	}
	
	APawn* MyPawn = GetPawn();
	APawn* TargetPlayer = CachedPlayerPawn.Get();

	if (!MyPawn || !TargetPlayer || !TargetPlayer) return;

	// 1. 플레이어와 이 AI 폰 사이의 거리 계산
	const float DistanceToPlayer = FVector::Distance(MyPawn->GetActorLocation(), TargetPlayer->GetActorLocation());

	if (!StateTreeComponent) return;

	const bool bShouldPauseByDistance = DistanceToPlayer > SleepDistanceThreshold;
	if (bPausedByDistance != bShouldPauseByDistance)
	{
		bPausedByDistance = bShouldPauseByDistance;
		RefreshLogicPauseState();
	}
}

void AECEnemyAIController::RefreshLogicPauseState()
{
	if (bIsPawnDead || !IsValid(StateTreeComponent))
	{
		return;
	}

	const bool bShouldPause = bPausedByDistance || bPausedForCinematic;

	if (bShouldPause)
	{
		if (StateTreeComponent->IsRunning() && !StateTreeComponent->IsPaused())
		{
			StateTreeComponent->PauseLogic(
				bPausedForCinematic
					? TEXT("Cutscene in progress")
					: TEXT("AI LOD distance exceeded"));
		}

		// StateTree만 일시정지하고 Pawn/Animation Tick은 유지합니다.
		// Actor Tick을 끄면 이동 컴포넌트와 애니메이션 업데이트 시점이 어긋나
		// 캐릭터가 기본 포즈로 이동하는 현상이 발생할 수 있습니다.
		return;
	}

	if (StateTreeComponent->IsPaused())
	{
		StateTreeComponent->ResumeLogic(TEXT("All AI pause reasons cleared"));
	}
}

void AECEnemyAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	if (bIsPawnDead || bPausedForCinematic)
	{
		return;
	}
	
	// 1. 들어온 액터가 플레이어 캐릭터인지 캐스팅을 통해 확인
	if (AECPlayer* TargetPlayer = Cast<AECPlayer>(Actor))
	{
		// 2. 발견했는지, 놓쳤는지 확인
		if (Stimulus.WasSuccessfullySensed())
		{
			const bool bHadLineOfSight = bHasLineOfSight;
			EngageTarget(TargetPlayer);

			if (!bHadLineOfSight)
			{
				UE_LOG(LogTemp, Warning, TEXT("플레이어를 발견했습니다."));
			}
		}
		else
		{
			if (TargetActor != TargetPlayer)
			{
				return;
			}
			
			LastKnownTargetLocation = Stimulus.StimulusLocation;
			if (IsActionMontagePlaying())
			{
				bLostSightDeferredByActionMontage = true;
			}

			if (LostSightGracePeriod <= 0.0f)
			{
				HandleLostSightGraceExpired();
			}
			else
			{
				GetWorldTimerManager().SetTimer(
					LostSightTimerHandle,
					this,
					&ThisClass::HandleLostSightGraceExpired,
					LostSightGracePeriod,
					false);
			}
		}
	}
}

void AECEnemyAIController::HandleLostSightGraceExpired()
{
	GetWorldTimerManager().ClearTimer(LostSightTimerHandle);

	if (bIsPawnDead || bPausedForCinematic || !IsValid(TargetActor))
	{
		return;
	}

	// 공격 도중 시야를 잃었다고 TargetActor를 지우면 공격 완료 직후 Patrol로 복귀합니다.
	// 현재 액션이 끝날 때까지 전투 대상을 유지하고 짧게 재검사합니다.
	if (IsActionMontagePlaying())
	{
		bLostSightDeferredByActionMontage = true;
		GetWorldTimerManager().SetTimer(
			LostSightTimerHandle,
			this,
			&ThisClass::HandleLostSightGraceExpired,
			0.1f,
			false);
		return;
	}

	// 몽타주가 끝난 프레임에 바로 전투를 해제하지 않고, 기존 시야 상실 유예시간을 다시 부여합니다.
	// 이 사이 StateTree가 살아 있는 TargetActor를 기준으로 회전/추적하면 Sight가 다시 갱신됩니다.
	if (bLostSightDeferredByActionMontage)
	{
		bLostSightDeferredByActionMontage = false;
		if (LostSightGracePeriod > 0.0f)
		{
			GetWorldTimerManager().SetTimer(
				LostSightTimerHandle,
				this,
				&ThisClass::HandleLostSightGraceExpired,
				LostSightGracePeriod,
				false);
			return;
		}
	}

	bHasLineOfSight = false;
	UE_LOG(LogTemp, Warning, TEXT("플레이어를 놓쳤습니다."));
	ClearCombatTarget();
}

bool AECEnemyAIController::IsActionMontagePlaying() const
{
	const ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn());
	const USkeletalMeshComponent* Mesh = ControlledCharacter ? ControlledCharacter->GetMesh() : nullptr;
	const UAnimInstance* AnimInstance = Mesh ? Mesh->GetAnimInstance() : nullptr;
	return AnimInstance && AnimInstance->IsAnyMontagePlaying();
}

void AECEnemyAIController::ClearCombatTarget()
{
	GetWorldTimerManager().ClearTimer(LostSightTimerHandle);
	bLostSightDeferredByActionMontage = false;
	ClearFocus(EAIFocusPriority::Gameplay);

	TargetActor = nullptr;
	bHasLineOfSight = false;
	bIsInCombat = false;
	LastKnownTargetLocation = FAISystem::InvalidLocation;

	UE_LOG(LogTemp, Warning, TEXT("전투 대상을 포기했습니다."));
}
