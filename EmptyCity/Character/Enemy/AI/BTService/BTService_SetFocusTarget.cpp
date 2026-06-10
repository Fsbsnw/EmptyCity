// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/AI/BTService/BTService_SetFocusTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_SetFocusTarget::UBTService_SetFocusTarget()
{
	NodeName = TEXT("Focus Target");

	// Service Tick 주기
	Interval = 0.1f;
	RandomDeviation = 0.f;
}

void UBTService_SetFocusTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();

	if (!IsValid(AIController))
	{
		return;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!IsValid(BlackboardComp))
	{
		return;
	}

	// Blackboard에 저장된 현재 타겟 액터를 가져옵니다.
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(GetSelectedBlackboardKey()));

	if (!IsValid(TargetActor))
	{
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
		return;
	}
	
	// Strafing 중에도 타겟을 계속 바라보도록 Focus를 갱신합니다.
	AIController->SetFocus(TargetActor, EAIFocusPriority::Gameplay);
}
