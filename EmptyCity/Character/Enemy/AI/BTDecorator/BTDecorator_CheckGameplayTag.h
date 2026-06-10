// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckGameplayTag.generated.h"

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UBTDecorator_CheckGameplayTag : public UBTDecorator
{
	GENERATED_BODY()
public:
	UBTDecorator_CheckGameplayTag();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	/** Decorator가 포함된 구간에 진입할 때 실행되는 함수입니다. */
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** Decorator가 포함된 구간을 벗어날 때 실행되는 함수입니다. */
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** GameplayTag가 변화될 때 호출되는 함수입니다. */
	void OnGameplayTagChanged(const FGameplayTag Tag, int32 NewCount, TWeakObjectPtr<UBehaviorTreeComponent> OwnerComp);

public:
	/** 에디터에서 설정할 태그입니다. */
	UPROPERTY(EditAnywhere, Category = "GameplayTag")
	FGameplayTag TagToCheck;
};
