// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECStorageInventoryWidget.generated.h"

class UButton;
class UDragDropOperation;
class UECInventoryItemInstance;
class UECInventorySlotWidget;
class UECItemDetailWidget;
class UECItemQuantitySelectorWidget;
class UInventoryViewModel;
class UOverlay;
class UWidget;
class UWrapBox;
struct FInventoryEntry;

/** 플레이어와 보관함 인벤토리 간의 아이템 이동 화면을 관리합니다. */
UCLASS()
class EMPTYCITY_API UECStorageInventoryWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	virtual void InjectViewModel(UMVVMViewModelBase* TargetViewModel) override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void OnWidgetOpened_Implementation() override;

	/** 플레이어나 보관함 인벤토리가 변경될 때마다 위젯을 업데이트합니다. */
	void RefreshInventoryUI();

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool NativeOnDrop(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	/** 플레이어의 Tool과 Weapon 카테고리 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> InventoryWrapBox_Player_ToolWeapon;

	/** 플레이어의 Stimpak과 Stimpill 카테고리 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> InventoryWrapBox_Player_SurvivalKit;

	/** 플레이어의 나머지 일반 아이템 카테고리 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> InventoryWrapBox_Player_Item;

	/** 보관함 인벤토리 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> InventoryWrapBox_Container;

	/** 플레이어 인벤토리 전체 드롭 영역입니다. 없으면 세 플레이어 WrapBox 영역을 사용합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> DropTarget_PlayerInventory;

	/** 보관함 전체 드롭 영역입니다. 없으면 보관함 WrapBox 영역을 사용합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> DropTarget_StorageInventory;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_MoveAllItemsToContainer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_MoveSelectedItemToContainer;

	/** 보관함 아이템 선택 시 왼쪽 상세 영역에서 사용할 수량 선택 위젯입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UECItemQuantitySelectorWidget> ItemQuantitySelector_Left;

	/** 플레이어 아이템 선택 시 오른쪽 상세 영역에서 사용할 수량 선택 위젯입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UECItemQuantitySelectorWidget> ItemQuantitySelector_Right;

	/** 새 좌/우 선택기를 배치하기 전까지 사용하는 기존 호환 위젯입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UECItemQuantitySelectorWidget> ItemQuantitySelector;

	/** 보관함 아이템 선택 시 플레이어 인벤토리 쪽(왼쪽)에 표시할 상세 영역입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> Overlay_ItemDetail_Left;

	/** 플레이어 아이템 선택 시 보관함 쪽(오른쪽)에 표시할 상세 영역입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> Overlay_ItemDetail_Right;

	/** 보관함 아이템의 상세 정보를 왼쪽에 표시합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UECItemDetailWidget> ItemDetailWidget_Left;

	/** 플레이어 아이템의 상세 정보를 오른쪽에 표시합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UECItemDetailWidget> ItemDetailWidget_Right;

	/** 새 좌/우 상세 위젯을 아직 배치하지 않은 기존 블루프린트를 위한 호환 바인딩입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> Overlay_ItemDetail;

	/** 새 좌/우 상세 위젯을 아직 배치하지 않은 기존 블루프린트를 위한 호환 바인딩입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UECItemDetailWidget> ItemDetailWidget;

	/** 인벤토리 슬롯 위젯에 사용할 블루프린트 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UECInventorySlotWidget> SlotWidgetClass;

private:
	enum class ESelectedInventorySource : uint8
	{
		None,
		Player,
		Storage
	};

	void RefreshInventoryWrapBox(
		UWrapBox* TargetWrapBox,
		const TArray<FInventoryEntry>& Entries,
		ESelectedInventorySource InventorySource);
	void RefreshSelectionVisuals();
	void RefreshSelectedItemDetail();
	void ClearItemDetailWidgets();
	UECItemDetailWidget* GetSelectedItemDetailWidget() const;
	UOverlay* GetSelectedItemDetailOverlay() const;
	UECItemQuantitySelectorWidget* GetSelectedItemQuantitySelector() const;
	void ClearItemQuantitySelectors();
	void UnbindInventoryViewModels();

	void HandlePlayerSlotClicked(UECInventoryItemInstance* ItemInstance);
	void HandleStorageSlotClicked(UECInventoryItemInstance* ItemInstance);
	void HandlePlayerSlotRightClicked(UECInventoryItemInstance* ItemInstance);
	void HandleStorageSlotRightClicked(UECInventoryItemInstance* ItemInstance);
	int32 GetSelectedItemStackCount() const;
	int32 GetSelectedActionQuantity() const;
	ESelectedInventorySource ResolveDropTarget(const FVector2D& ScreenPosition) const;

	UFUNCTION()
	void HandleMoveAllItemsToContainerClicked();

	UFUNCTION()
	void HandleMoveSelectedItemClicked();

	void ClearSelectedItem();

	UPROPERTY()
	TObjectPtr<UECInventoryItemInstance> SelectedItemInstance;

	UPROPERTY()
	TObjectPtr<UInventoryViewModel> CachedPlayerInventoryViewModel;

	UPROPERTY()
	TObjectPtr<UInventoryViewModel> CachedStorageInventoryViewModel;

	ESelectedInventorySource SelectedInventorySource = ESelectedInventorySource::None;
};
