// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/AI/ECEnemyAIController.h"

#include "ECGameplayTags.h"
#include "Character/Enemy/ECEnemyCharacterBase.h"
#include "Character/Player/ECPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "StateTree.h"

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

	UE_LOG(LogTemp, Log,
		TEXT("%s: %s에 지정된 StateTree %s를 시작했습니다."),
		*GetNameSafe(this),
		*GetNameSafe(EnemyCharacter->GetClass()),
		*GetNameSafe(StateTreeAsset));
}

void AECEnemyAIController::OnUnPossess()
{
	if (IsValid(StateTreeComponent) && StateTreeComponent->IsRunning())
	{
		StateTreeComponent->StopLogic(TEXT("Enemy pawn unpossessed"));
	}

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

	GetWorldTimerManager().ClearTimer(LODTimerHandle);

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

	// 2. 거리에 따른 조건 분기
	if (DistanceToPlayer > SleepDistanceThreshold)
	{
		// 일정 거리보다 멀어지면 StateTree를 일시정지(Pause) 시킵니다.
		if (StateTreeComponent->IsRunning())
		{
			StateTreeComponent->PauseLogic(TEXT("플레이어와의 거리가 최적화 범위 밖으로 벗어났습니다 - Sleep"));
			UE_LOG(LogTemp, Warning, TEXT("플레이어와의 거리가 최적화 범위 밖으로 벗어났습니다 - Sleep"));
            
			// 필요하다면 여기서 액터의 틱이나 애니메이션 틱도 완전히 꺼버릴 수 있습니다.
			MyPawn->SetActorTickEnabled(false);
			
			/* 충분히 멀어진 경우, 타이머 주기도 늘리는 것도 고려. 예시 : 4초
			 * GetWorldTimerManager().SetTimer(LODTimerHandle, this, &ThisClass::UpdateAILOD, 4.0f, true, InitialDelay);
			 *
			 */
		}
	}
	else
	{
		// 다시 범위 안으로 들어오면 StateTree를 재개(Resume)합니다.
		if (StateTreeComponent->IsPaused())
		{
			StateTreeComponent->ResumeLogic(TEXT("플레이어와의 거리가 최적화 범위 안으로 들어왔습니다 - Wake Up"));
			UE_LOG(LogTemp, Warning, TEXT("플레이어와의 거리가 최적화 범위 안으로 들어왔습니다 - Wake Up"));
			MyPawn->SetActorTickEnabled(true);
		}
	}
}

void AECEnemyAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	if (bIsPawnDead)
	{
		return;
	}
	
	// 1. 들어온 액터가 플레이어 캐릭터인지 캐스팅을 통해 확인
	if (AECPlayer* TargetPlayer = Cast<AECPlayer>(Actor))
	{
		// 2. 발견했는지, 놓쳤는지 확인
		if (Stimulus.WasSuccessfullySensed())
		{
			const bool bWasAlreadyInCombat = bIsInCombat;
			
			bHasLineOfSight = true;
			bIsInCombat = true;
			LastKnownTargetLocation = TargetPlayer->GetActorLocation();
			
			// 플레이어를 발견함 (블랙보드 TargetActor에 Player 등록)
			TargetActor = TargetPlayer;
			
			if (!bWasAlreadyInCombat && IsValid(StateTreeComponent) && StateTreeComponent->IsRunning())
			{
				FStateTreeEvent Event;
				Event.Tag = ECGameplayTags::StateTree_Event_Engage;
				StateTreeComponent->SendStateTreeEvent(Event);
			}
			UE_LOG(LogTemp, Warning, TEXT("플레이어를 발견했습니다."));
		}
		else
		{
			if (TargetActor != TargetPlayer)
			{
				return;
			}
			
			bHasLineOfSight = false;
			LastKnownTargetLocation = Stimulus.StimulusLocation;
			
			UE_LOG(LogTemp, Warning, TEXT("플레이어를 놓쳤습니다."));
		}
	}
}

void AECEnemyAIController::ClearCombatTarget()
{
	TargetActor = nullptr;
	bHasLineOfSight = false;
	bIsInCombat = false;
	LastKnownTargetLocation = FAISystem::InvalidLocation;

	UE_LOG(LogTemp, Warning, TEXT("전투 대상을 포기했습니다."));
}
