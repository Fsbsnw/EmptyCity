#pragma once

#include "CoreMinimal.h"
#include "ECWorkbenchUpgradeTypes.generated.h"

/** 플레이어 단위로 저장되는 작업대 강화 대상입니다. */
UENUM(BlueprintType)
enum class EECWorkbenchUpgradeType : uint8
{
	MaxHealth			UMETA(DisplayName = "Max Health"),
	MaxStamina			UMETA(DisplayName = "Max Stamina"),
	MaxInventoryWeight	UMETA(DisplayName = "Max Inventory Weight"),
	Axe					UMETA(DisplayName = "Axe"),
	Pickaxe				UMETA(DisplayName = "Pickaxe"),
	Weapon				UMETA(DisplayName = "Weapon"),
	MAX					UMETA(Hidden)
};

