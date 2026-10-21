// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Character/Enemy/Component/EnemyMeleeTraceComponent.h"
#include "ECAnimNotifyState_EnemyAttack.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName="EC Enemy Attack Trace Window"))
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

	/** 이 공격 애니메이션의 무기가 지면에 닿았을 때 생성할 데칼 설정입니다. */
	UPROPERTY(EditAnywhere, Category="Weapon Ground Decal",
		meta=(EditCondition="TracePart == EEnemyMeleeTracePart::Weapon", EditConditionHides, AllowPrivateAccess="true"))
	FEnemyWeaponGroundDecalConfig WeaponGroundDecalConfig;
};
