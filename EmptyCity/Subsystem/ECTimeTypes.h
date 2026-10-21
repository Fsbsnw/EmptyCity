#pragma once

#include "CoreMinimal.h"
#include "ECTimeTypes.generated.h"

UENUM(BlueprintType)
enum class EECTimeOfDay : uint8
{
	Morning   UMETA(DisplayName = "Morning"),
	Afternoon UMETA(DisplayName = "Afternoon"),
	Evening   UMETA(DisplayName = "Evening")
};
