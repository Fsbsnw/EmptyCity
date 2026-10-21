// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECPlayerInventoryWidget.generated.h"

class UECInventoryItemInstance;
class UECItemQuantitySelectorWidget;
class UECInventorySlotWidget;
class UECItemDetailWidget;
class UInventoryViewModel;
class UOverlay;
class UScrollBox;
class UWrapBox;
struct FInventoryEntry;

/** 플레이어의 단독 인벤토리 화면을 관리합니다. */
UCLASS()
class EMPTYCITY_API UECPlayerInventoryWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	virtual void InjectViewModel(UMVVMViewModelBase* TargetViewModel) override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void OnWidgetOpened_Implementation() override;

	/** 플레이어 인벤토리가 변경될 때마다 위젯을 업데이트합니다. */
	void RefreshInventoryUI();

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> InventoryScrollBox_Player;

	/** Tool과 Weapon을 함께 표시하는 카테고리 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> InventoryWrapBox_Player_ToolWeapon;

	/** Stimpak과 Stimpill을 표시하는 카테고리 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> InventoryWrapBox_Player_SurvivalKit;

	/** 나머지 일반 아이템을 표시하는 카테고리 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> InventoryWrapBox_Player_Item;

	/** G키로 버릴 수량을 선택하는 공통 위젯입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UECItemQuantitySelectorWidget> ItemQuantitySelector;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> Overlay_ItemDetail;

	/** 선택한 아이템의 공통 상세 정보를 표시합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UECItemDetailWidget> ItemDetailWidget;

	/** 인벤토리 슬롯 위젯에 사용할 블루프린트 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UECInventorySlotWidget> SlotWidgetClass;

private:
	void RefreshInventoryWrapBox(UWrapBox* TargetWrapBox, const TArray<FInventoryEntry>& Entries);
	void RefreshSelectionVisuals();
	void UnbindInventoryViewModel();

	void HandleInventorySlotClicked(UECInventoryItemInstance* ItemInstance);
	int32 GetSelectedItemStackCount() const;
	int32 GetSelectedActionQuantity() const;

	void HandleDiscardSelectedItem();

	void ClearSelectedItem();

	UPROPERTY()
	TObjectPtr<UECInventoryItemInstance> SelectedItemInstance;

	UPROPERTY()
	TObjectPtr<UInventoryViewModel> CachedPlayerInventoryViewModel;
};
