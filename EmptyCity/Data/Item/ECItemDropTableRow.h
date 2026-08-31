#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ECItemDropTableRow.generated.h"

/** 한 종류의 아이템에 대한 드롭 수량과 확률을 정의합니다. */
USTRUCT()
struct FECItemDropEntry
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Drop Entry
// ─────────────────────────────────────────────────────────────
public:
	/** 드롭할 DT_Item 행입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|드롭", meta = (DisplayName = "아이템", RowType = "/Script/EmptyCity.ECItemTableRow"))
	FDataTableRowHandle Item;

	/** 드롭할 최소 수량입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|드롭", meta = (DisplayName = "최소 수량", ClampMin = "1"))
	int32 MinCount = 1;

	/** 드롭할 최대 수량입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|드롭", meta = (DisplayName = "최대 수량", ClampMin = "1"))
	int32 MaxCount = 1;

	/** 드롭 확률입니다. 100이면 항상 드롭합니다. */
	UPROPERTY(EditAnywhere, Category = "변수|드롭", meta = (DisplayName = "드롭 확률", ClampMin = "0", ClampMax = "100", Units = "%"))
	int32 DropChance = 100;
};

/** 하나의 드롭 대상이 생성할 아이템 목록을 정의하는 공통 행 구조체입니다. */
USTRUCT()
struct FECItemDropTableRow : public FTableRowBase
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Drop Table
// ─────────────────────────────────────────────────────────────
public:
	/** 각각 독립적으로 확률과 수량을 계산할 드롭 목록입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|드롭", meta = (DisplayName = "드롭 목록"))
	TArray<FECItemDropEntry> DropEntries;
};
