#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "ECGameplayCue_ParryImpact.generated.h"

class UECCombatFeedbackDataAsset;

/** 패링 성공 접촉점의 VFX와 SFX를 CombatFeedback 데이터로 실행합니다. */
UCLASS()
class EMPTYCITY_API UECGameplayCue_ParryImpact : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(
		AActor* MyTarget,
		const FGameplayCueParameters& Parameters) const override;

protected:
	/** 기존 타격 피드백과 동일한 전투 피드백 데이터 에셋을 사용합니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Feedback Data")
	TObjectPtr<const UECCombatFeedbackDataAsset> FeedbackDataAsset;
};
