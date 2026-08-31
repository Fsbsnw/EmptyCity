// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notify/ECAnimNotifyState_EnemyAttack.h"

#include "Character/Enemy/Component/EnemyMeleeTraceComponent.h"

UECAnimNotifyState_EnemyAttack::UECAnimNotifyState_EnemyAttack()
{
	bIsNativeBranchingPoint = true;
}

void UECAnimNotifyState_EnemyAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UEnemyMeleeTraceComponent* TraceComponent =	OwnerActor->FindComponentByClass<UEnemyMeleeTraceComponent>())
		{
			TraceComponent->BeginTrace(TracePart);
		}
	}
}

void UECAnimNotifyState_EnemyAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UEnemyMeleeTraceComponent* TraceComponent =	OwnerActor->FindComponentByClass<UEnemyMeleeTraceComponent>())
		{
			TraceComponent->EndTrace();
		}
	}
}
