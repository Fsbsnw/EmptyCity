// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/AI/ECEnemyAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/ECEnemyCharacterBase.h"
#include "Character/Player/ECPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

AECEnemyAIController::AECEnemyAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComponent");
	check(Blackboard);
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComponent");
	check(BehaviorTreeComponent);

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
	
	BrainComponent = BehaviorTreeComponent;
}

void AECEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Behavior Tree를 활성화시킵니다.
	if (AECEnemyCharacterBase* EnemyCharacter = Cast<AECEnemyCharacterBase>(InPawn))
	{
		if (UBehaviorTree* BTAsset = EnemyCharacter->GetBehaviorTree())
		{
			UE_LOG(LogTemp, Warning, TEXT("BT : %s"),*BTAsset->GetName());
			RunBehaviorTree(BTAsset);
		}
	}
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

	// 초기 위치를 저장하고 주변 반경 안에서 이동합니다.
	if (Blackboard && Blackboard->GetBlackboardAsset())
	{
		Blackboard->SetValueAsVector("InitialLocation", GetPawn()->GetActorLocation());
	}
}

void AECEnemyAIController::UpdateAILOD()
{
	APawn* MyPawn = GetPawn();
	APawn* TargetPlayer = CachedPlayerPawn.Get();

	if (!MyPawn || !TargetPlayer || !TargetPlayer) return;

	// 1. 플레이어와 이 AI 폰 사이의 거리 계산
	const float DistanceToPlayer = FVector::Distance(MyPawn->GetActorLocation(), TargetPlayer->GetActorLocation());

	// BT를 제어하는 BrainComponent를 가져옵니다.
	if (!BrainComponent) return;

	// 2. 거리에 따른 조건 분기
	if (DistanceToPlayer > SleepDistanceThreshold)
	{
		// 일정 거리보다 멀어지면 비헤이비어 트리를 일시정지(Pause) 시킵니다.
		if (BrainComponent->IsRunning())
		{
			BrainComponent->PauseLogic(TEXT("플레이어와의 거리가 최적화 범위 밖으로 벗어났습니다 - Sleep"));
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
		// 다시 범위 안으로 들어오면 비헤이비어 트리를 재개(Resume)합니다.
		if (BrainComponent->IsPaused())
		{
			BrainComponent->ResumeLogic(TEXT("플레이어와의 거리가 최적화 범위 안으로 들어왔습니다 - Wake Up"));
			UE_LOG(LogTemp, Warning, TEXT("플레이어와의 거리가 최적화 범위 안으로 들어왔습니다 - Wake Up"));
			MyPawn->SetActorTickEnabled(true);
		}
	}
}

void AECEnemyAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	// 임시 구현, 플레이어/반려동물 체크 기능 구현 필요
	if (!Blackboard || !Blackboard->GetBlackboardAsset()) 
	{
		return; 
	}
	
	// 1. 들어온 액터가 플레이어 캐릭터인지 캐스팅을 통해 확인
	if (AECPlayer* TargetPlayer = Cast<AECPlayer>(Actor))
	{
		// 2. 발견했는지, 놓쳤는지 확인
		if (Stimulus.WasSuccessfullySensed())
		{
			// 플레이어를 발견함 (블랙보드 TargetActor에 Player 등록)
			Blackboard->SetValueAsObject("TargetActor", TargetPlayer);
			UE_LOG(LogTemp, Warning, TEXT("플레이어를 발견했습니다."));
		}
		else
		{
			// 플레이어를 놓침 (블랙보드 TargetActor 비우기)
			Blackboard->ClearValue("TargetActor");
			UE_LOG(LogTemp, Warning, TEXT("플레이어를 놓쳤습니다."));
		}
	}
}
