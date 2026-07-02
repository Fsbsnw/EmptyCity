// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECInventoryManagerComponent.h"
#include "UObject/Interface.h"
#include "IECInventory.generated.h"

class UECInventoryManagerComponent;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIECInventory : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class EMPTYCITY_API IIECInventory
{
	GENERATED_BODY()
public:
	virtual UECInventoryManagerComponent* GetInventoryManagerComponent() const = 0;
};