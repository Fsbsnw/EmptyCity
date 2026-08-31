#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Inventory/IPickupable.h"
#include "ECItemDropComponent.generated.h"

/** 지정한 아이템 드롭 행을 계산해 아이템 드롭 결과를 생성하는 공통 컴포넌트입니다. */
UCLASS()
class EMPTYCITY_API UECItemDropComponent : public UActorComponent
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// ActorComponent Interface
// ─────────────────────────────────────────────────────────────
public:
	UECItemDropComponent();

	
// ─────────────────────────────────────────────────────────────
// Drop
// ─────────────────────────────────────────────────────────────
public:
	/** 지정된 드롭 행을 계산해 소유 액터 주변에 픽업 액터로 생성합니다. */
	void SpawnDrop() const;

private:
	/** 지정된 드롭 행의 확률과 수량을 계산해 픽업 내용을 반환합니다. */
	FInventoryPickup GenerateDrop() const;

	
// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
protected:
	/** 이 컴포넌트가 사용할 적 또는 파밍 대상의 드롭 행입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|드롭", meta = (RowType = "/Script/EmptyCity.ECItemDropTableRow"))
	FDataTableRowHandle DropTableRow;
	
};
