// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "InventoryFragment_Unlockable.generated.h"

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UInventoryFragment_Unlockable : public UECInventoryItemFragment
{
	GENERATED_BODY()

public:
	/** 이 아이템을 해금하기 위해 필요한 태그. 비어있으면 기본 지급 아이템. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unlock")
	FGameplayTag RequiredUnlockTag;
};
