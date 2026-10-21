#pragma once

#include "CoreMinimal.h"
#include "ECQuickSlotTypes.generated.h"

class UTexture2D;

/** QuickBar 슬롯 하나를 화면에 표시하기 위한 데이터입니다. */
USTRUCT(BlueprintType)
struct EMPTYCITY_API FECQuickSlotViewData
{
	GENERATED_BODY()

	/** DT_Item에 등록된 아이템 아이콘입니다. */
	UPROPERTY(BlueprintReadOnly, Category = "Quick Slot")
	TSoftObjectPtr<UTexture2D> ItemIcon;

	/** 같은 Item Definition에 해당하는 전체 인벤토리 재고입니다. */
	UPROPERTY(BlueprintReadOnly, Category = "Quick Slot")
	int32 StockCount = 0;

	/** QuickBar 슬롯에 아이템 종류가 지정되어 있는지 나타냅니다. */
	UPROPERTY(BlueprintReadOnly, Category = "Quick Slot")
	bool bAssigned = false;

	/** 현재 재고가 있어 슬롯 아이템을 사용할 수 있는지 나타냅니다. */
	UPROPERTY(BlueprintReadOnly, Category = "Quick Slot")
	bool bCanUse = false;

	/** 가장 최근 사용에 성공한 슬롯인지 나타냅니다. WBP에서 흑백/컬러 이미지를 교체할 때 사용합니다. */
	UPROPERTY(BlueprintReadOnly, Category = "Quick Slot")
	bool bIsLastUsed = false;

	bool operator==(const FECQuickSlotViewData& Other) const
	{
		return ItemIcon == Other.ItemIcon
			&& StockCount == Other.StockCount
			&& bAssigned == Other.bAssigned
			&& bCanUse == Other.bCanUse
			&& bIsLastUsed == Other.bIsLastUsed;
	}

	bool operator!=(const FECQuickSlotViewData& Other) const
	{
		return !(*this == Other);
	}
};
