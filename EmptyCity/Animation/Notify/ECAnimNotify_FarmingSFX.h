#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ECAnimNotify_FarmingSFX.generated.h"

/**
 * 파밍 애니메이션의 지정 프레임에서 DT_Sound의 SFX를 재생합니다.
 * 재생할 행 이름은 현재 파밍 대상의 DT_Farming 데이터에서 가져옵니다.
 */
UCLASS(meta = (DisplayName = "EC Farming SFX"))
class EMPTYCITY_API UECAnimNotify_FarmingSFX : public UAnimNotify
{
	GENERATED_BODY()

public:
	UECAnimNotify_FarmingSFX(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};
