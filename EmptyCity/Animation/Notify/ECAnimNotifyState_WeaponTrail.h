#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ECAnimNotifyState_WeaponTrail.generated.h"

/**
 * 무기의 Trail VFX를 활성화하는 구간입니다.
 */
UCLASS(meta=(DisplayName="EC Weapon Trail Window"))
class EMPTYCITY_API UECAnimNotifyState_WeaponTrail : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UECAnimNotifyState_WeaponTrail();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};