// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ECEnemyAnimInstance.generated.h"

struct FGameplayTag;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUninitializeAnimation() override;
	
// ─────────────────────────────────────────────────────────────
// AnimBlueprint Variables
// ─────────────────────────────────────────────────────────────
public:
	/** 스턴 상태를 체크하는 변수입니다. */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsStunned = false;

protected:
	/** 스턴 상태가 변화할 때 실행되는 함수입니다. */
	void OnStunTagChanged(const FGameplayTag Tag, int32 NewCount);
};
