#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ECAnimNotify_BossPatternDetonation.generated.h"

/** 스크림 애니메이션의 지정 프레임에서 준비된 보스 장판의 순차 폭발을 시작합니다. */
UCLASS(meta=(DisplayName="EC Boss Pattern Start Detonation"))
class EMPTYCITY_API UECAnimNotify_BossPatternDetonation : public UAnimNotify
{
	GENERATED_BODY()

public:
	UECAnimNotify_BossPatternDetonation(
		const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};
