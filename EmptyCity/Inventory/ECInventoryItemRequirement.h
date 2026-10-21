#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "ECInventoryItemRequirement.generated.h"

class UECInventoryItemDefinition;

/**
 * 아이템 설계도와 필요한 수량을 함께 나타내는 공통 재료 단위입니다.
 * 같은 설계도가 여러 번 들어오면 인벤토리 트랜잭션에서 하나로 합산됩니다.
 */
USTRUCT(BlueprintType)
struct EMPTYCITY_API FECInventoryItemRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowAbstract = "false"))
	TSubclassOf<UECInventoryItemDefinition> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1", UIMin = "1"))
	int32 Count = 1;
};
