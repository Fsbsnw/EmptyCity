// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Character/Enemy/Component/EnemyMeleeTraceComponent.h"
#include "ECAnimNotifyState_EnemyAttack.generated.h"

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECAnimNotifyState_EnemyAttack : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UECAnimNotifyState_EnemyAttack();
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	/** 이 Notify 구간에서 판정할 공격 부위입니다. */
	UPROPERTY(EditAnywhere, Category="Trace", meta=(AllowPrivateAccess="true"))
	EEnemyMeleeTracePart TracePart = EEnemyMeleeTracePart::Weapon;
};
