#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ECAnimNotify_CameraShake.generated.h"

class UCameraShakeBase;

/** 애니메이션의 타격 시점에 로컬 플레이어 카메라 셰이크를 재생합니다. */
UCLASS(meta = (DisplayName = "EC Camera Shake"))
class EMPTYCITY_API UECAnimNotify_CameraShake : public UAnimNotify
{
	GENERATED_BODY()

public:
	UECAnimNotify_CameraShake(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
private:
	/** 재생할 카메라 셰이크 클래스입니다. */
	UPROPERTY(EditAnywhere, Category = "Camera Shake", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> ShakeClass;

	/** 카메라 셰이크의 재생 강도입니다. */
	UPROPERTY(EditAnywhere, Category = "Camera Shake", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float Scale = 1.0f;
};
