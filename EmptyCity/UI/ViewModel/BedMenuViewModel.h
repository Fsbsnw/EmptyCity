// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "BedMenuViewModel.generated.h"

class AECBed;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UBedMenuViewModel : public UECViewModelBase
{
	GENERATED_BODY()
public:
	virtual void BindCallbacksToDependencies(AActor* ContextActor) override;

	/** 수면을 실행합니다. */
	UFUNCTION(BlueprintCallable)
	void RequestSleepSequence();

private:
	TObjectPtr<AECBed> TargetBed = nullptr;
};
