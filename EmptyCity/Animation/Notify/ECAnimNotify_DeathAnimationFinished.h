#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ECAnimNotify_DeathAnimationFinished.generated.h"

/** 적 사망 애니메이션 종료를 DeathVisualComponent에 전달합니다. */
UCLASS(meta = (DisplayName = "EC Death Animation Finished"))
class EMPTYCITY_API UECAnimNotify_DeathAnimationFinished : public UAnimNotify
{
	GENERATED_BODY()

public:
	UECAnimNotify_DeathAnimationFinished(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
