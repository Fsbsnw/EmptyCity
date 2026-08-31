#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ECAnimNotifyState_AttackWindow.generated.h"

/** 몽타주의 공격 판정 구간 시작과 종료를 현재 장착 무기에 전달합니다. */
UCLASS(meta = (DisplayName = "EC Attack Window"))
class EMPTYCITY_API UECAnimNotifyState_AttackWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UECAnimNotifyState_AttackWindow(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
