#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ECAnimNotify_HitStop.generated.h"

/** 피격 애니메이션의 원하는 자세에서 대기 중인 히트스탑을 실행합니다. */
UCLASS(meta = (DisplayName = "EC Hit Stop"))
class EMPTYCITY_API UECAnimNotify_HitStop : public UAnimNotify
{
	GENERATED_BODY()

public:
	UECAnimNotify_HitStop(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
