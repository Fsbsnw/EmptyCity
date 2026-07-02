#pragma once

#include "Inventory/ECInventoryItemDefinition.h"
#include "Styling/SlateBrush.h"
#include "InventoryFragment_QuickBarIcon.generated.h"

class UObject;

UCLASS()
class UInventoryFragment_QuickBarIcon : public UECInventoryItemFragment
{
	GENERATED_BODY()
	
	/** 데이터테이블화 예비용 
public:
	virtual void PostLoad() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="변수", meta=(RowType="/Script/LyraGame.LyraQuickBarIconRow"))                                                                                                                                                              
	FDataTableRowHandle IconRow; */

public:
	/** 인벤토리 UI에 표시되는 아이템 이미지입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="변수")
	FSlateBrush Brush;
	
};
