// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECItemDetailWidget.generated.h"

class UECInventoryItemInstance;
class UImage;
class UTextBlock;
struct FECWeaponTableRow;

/**
 * 선택한 아이템의 공통 정보를 표시하는 위젯입니다.
 *
 * 가격은 화면마다 달라질 수 있으므로 호출하는 화면에서 전달받습니다.
 * 가격, 무게, 개수 TextBlock은 아직 배치되지 않은 화면에서도 사용할 수 있도록
 * 선택적 바인딩으로 선언합니다.
 */
UCLASS()
class EMPTYCITY_API UECItemDetailWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** 아이템 정보와 현재 화면에서 표시할 개수/가격을 적용합니다. */
	UFUNCTION(BlueprintCallable, Category = "Item Detail")
	void SetItem(UECInventoryItemInstance* ItemInstance, int32 StackCount = 1, int32 Price = 0);

	/** 표시 중인 아이템 정보를 비우고 위젯을 숨깁니다. */
	UFUNCTION(BlueprintCallable, Category = "Item Detail")
	void ClearItem();

	/** Workbench에서 선택한 무기의 DT_Weapon 정보와 해금 상태를 추가로 표시합니다. */
	void SetWeaponData(const FECWeaponTableRow* WeaponData, bool bIsUnlocked);

private:
	void ClearWeaponData();

	UPROPERTY()
	TObjectPtr<UECInventoryItemInstance> CachedItemInstance;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_ItemIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ItemName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ItemDescription;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ItemPrice;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ItemWeight;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ItemCount;

	/** "해금" 또는 "미해금"을 표시합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ItemUnlockState;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_WeaponAttackPower;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_WeaponAttackSpeed;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_WeaponLightStaminaCost;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_WeaponHeavyStaminaCost;
};
