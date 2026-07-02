// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "InventoryFragment_ItemSlotIcon.generated.h"

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UInventoryFragment_ItemSlotIcon : public UECInventoryItemFragment
{
	GENERATED_BODY()
		
	/** 데이터테이블화 예비용 
public:
	virtual void PostLoad() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="변수", meta=(RowType="/Script/LyraGame.LyraQuickBarIconRow"))                                                                                                                                                              
	FDataTableRowHandle IconRow; */

public:
	/** 아이템 슬롯에 표시되는 아이템 이름입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FText ItemName;

	/** 아이템 슬롯이 Hovered 상태일 때 표시되는 아이템 설명입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta=(MultiLine=true))
	FText ItemDescription;

	/** 아이템 슬롯이 Hovered 상태일 때 표시되는 아이템 설명입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta=(MultiLine=true))
	FText ItemLockedDescription;
	
	/** 인벤토리 UI에 표시되는 아이템 이미지입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="변수")
	FSlateBrush ItemBrush;

	/** 잠겨있는 아이템 이미지입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="변수")
	FSlateBrush ItemLockedBrush;
};
