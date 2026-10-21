#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Save/ECSaveTypes.h"
#include "ECSaveGame.generated.h"

UCLASS()
class EMPTYCITY_API UECSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Save")
	int32 SaveVersion = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Save")
	FECGameplaySaveData SaveData;
};
