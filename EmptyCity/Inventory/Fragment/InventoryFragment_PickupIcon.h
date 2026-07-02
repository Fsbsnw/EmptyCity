#pragma once

#include "Inventory/ECInventoryItemDefinition.h"
#include "UObject/ObjectPtr.h"
#include "InventoryFragment_PickupIcon.generated.h"

class UObject;
class USkeletalMesh;

UCLASS()
class UInventoryFragment_PickupIcon : public UECInventoryItemFragment
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
public:
	/** 드랍된 아이템을 표시하기 위한 메시입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="변수")
	TObjectPtr<USkeletalMesh> SkeletalMesh;
	
};
