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
	/**
	 * 지정된 드롭 행을 계산합니다.
	 * 월드 메시가 있는 아이템은 픽업 액터로 생성하고, 없는 아이템은 수령 대상의 인벤토리에 바로 추가합니다.
	 */
	void SpawnDrop(AActor* RecipientActor = nullptr) const;

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

	/** 소유 액터 중심에서 월드 아이템을 배치할 최소 수평 거리입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|드롭 위치", meta = (ClampMin = "0.0", Units = "cm"))
	float DropSpawnRadius = 75.0f;

	/** 두 월드 아이템의 중심 사이에 유지할 최소 간격입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|드롭 위치", meta = (ClampMin = "0.0", Units = "cm"))
	float MinimumDropSpacing = 50.0f;

	/** 소유 액터 루트 컴포넌트 하단에서 지면 트레이스를 시작할 추가 높이입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|드롭 위치", meta = (ClampMin = "0.0", Units = "cm"))
	float GroundTraceUpDistance = 10.0f;

	/** 소유 액터 루트 컴포넌트 하단에서 아래로 지면을 찾을 거리입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|드롭 위치", meta = (ClampMin = "0.0", Units = "cm"))
	float GroundTraceDownDistance = 500.0f;

	/** 메시 최하단과 지면 사이에 둘 최소 여유 거리입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|드롭 위치", meta = (ClampMin = "0.0", Units = "cm"))
	float GroundClearance = 2.0f;
};
