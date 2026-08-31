#pragma once

#include "CoreMinimal.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "InventoryFragment_WorldVisual.generated.h"

class UStaticMesh;

/**
 * 월드에 배치되거나 드롭된 아이템의 외형을 정의하는 Fragment입니다.
 * AECPickupableItem은 이 Fragment의 StaticMesh와 WorldScale을 읽어 월드 외형을 자동으로 설정합니다.
 */
UCLASS(DisplayName = "월드 외형")
class EMPTYCITY_API UInventoryFragment_WorldVisual : public UECInventoryItemFragment
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
public:
	/** 월드에 드롭된 아이템을 표시할 스태틱 메시입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "월드", meta = (DisplayName = "스태틱 메시"))
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	/** Pickup 액터의 MeshComponent에 적용할 상대 스케일입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "월드", meta = (DisplayName = "월드 스케일"))
	FVector WorldScale = FVector::OneVector;
};
