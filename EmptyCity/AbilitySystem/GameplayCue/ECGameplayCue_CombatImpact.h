// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "ECGameplayCue_CombatImpact.generated.h"

class UCombatFeedbackDataAsset;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECGameplayCue_CombatImpact : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
	
public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
	/** 전투 피드백 관련 데이터 에셋입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Feedback Data")
	TObjectPtr<const UCombatFeedbackDataAsset> FeedbackDataAsset;
};
