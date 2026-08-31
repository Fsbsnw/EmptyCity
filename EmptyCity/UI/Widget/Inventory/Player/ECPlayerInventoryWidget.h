// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECPlayerInventoryWidget.generated.h"

class UOverlay;
class UECInventoryItemInstance;
class UButton;
struct FInventoryEntry;
class UECItemDetailWidget;
class UECInventorySlotWidget;
class UWrapBox;
class UECInventoryManagerComponent;
class UInventoryViewModel;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECPlayerInventoryWidget : public UECUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void InjectViewModel(UMVVMViewModelBase* TargetViewModel) override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void OnWidgetOpened_Implementation() override;
	
	/** 인벤토리가 변경될 때마다 위젯을 업데이트 합니다. */
	void RefreshInventoryUI();

private:
	enum class ESelectedInventorySource : uint8
	{
		None,
		Player,
		Storage
	};

	/** 지정한 WrapBox에 인벤토리 엔트리를 표시합니다. */
	void RefreshInventoryWrapBox(UWrapBox* TargetWrapBox, const TArray<FInventoryEntry>& Entries, bool bCanMoveToContainer);
	void RefreshSelectionVisuals();
	void UnbindInventoryViewModels();

	void HandleInventorySlotClicked(UECInventoryItemInstance* ItemInstance);
	void HandleContainerInventorySlotClicked(UECInventoryItemInstance* ItemInstance);
	void HandleInventorySlotRightClicked(UECInventoryItemInstance* ItemInstance);
	void HandleStorageSlotRightClicked(UECInventoryItemInstance* ItemInstance);
	int32 GetSelectedItemStackCount() const;

	UFUNCTION()
	void HandleMoveAllItemsToContainerClicked();
	
	UFUNCTION()
	void HandleMoveSelectedItemToContainerClicked();

	UFUNCTION()
	void HandleDiscardSelectedItemClicked();

	void ClearSelectedItem();

	UPROPERTY()
	TObjectPtr<UECInventoryItemInstance> SelectedItemInstance;

	UPROPERTY()
	TObjectPtr<UInventoryViewModel> CachedPlayerInventoryViewModel;

	UPROPERTY()
	TObjectPtr<UInventoryViewModel> CachedStorageInventoryViewModel;

	ESelectedInventorySource SelectedInventorySource = ESelectedInventorySource::None;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> InventoryWrapBox_Player;

	/** 컨테이너 인벤토리 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> InventoryWrapBox_Container;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_MoveAllItemsToContainer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton>	Button_MoveSelectedItemToContainer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_DiscardSelectedItem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> Overlay_ItemDetail;
	
	/** 선택한 아이템의 공통 상세 정보를 표시합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UECItemDetailWidget> ItemDetailWidget;

	/** 인벤토리 슬롯 위젯에 사용할 블루프린트 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UECInventorySlotWidget> SlotWidgetClass;
};
