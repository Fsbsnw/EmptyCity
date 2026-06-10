// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/AI/BTService/BTService_CheckDistance.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_CheckDistance::UBTService_CheckDistance()
{
	NodeName = "Check Distance To Target";
}

void UBTService_CheckDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// 1. 블랙보드 컴포넌트 가져오기
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return;

	// 2. AI 캐릭터 정보 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn) return;

	// 3. 블랙보드에서 타겟 오브젝트를 가져와 AActor로 캐스팅
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("TargetActor")));
	if (!TargetActor) 
	{
		BlackboardComp->ClearValue(BlackboardKey.SelectedKeyName);
		return;
	}

	// 4. 거리를 계산합니다.
	float Distance = FVector::Distance(AIPawn->GetActorLocation(), TargetActor->GetActorLocation());

	// 5. 계산된 거리를 블랙보드에 저장합니다.
	BlackboardComp->SetValueAsFloat(BlackboardKey.SelectedKeyName, Distance);
}