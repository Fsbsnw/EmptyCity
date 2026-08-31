// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECWorkbenchWidget.generated.h"

struct FInventoryEntry;
struct FECWeaponTableRow;
class UButton;
class UDataTable;
class UECInventoryItemInstance;
class UECInventorySlotWidget;
class UECItemDetailWidget;
class UOverlay;
class UScrollBox;
class UTextBlock;

UENUM(BlueprintType)
enum class EWorkbenchMode : uint8
{
	Stat,
	Tool,
	Weapon
};

/**
 * 스탯 강화, 도구 제작, 무기 제작 화면의 공통 UI를 관리합니다.
 * 실제 강화/제작 데이터와 실행 로직은 추후 전용 ViewModel에 연결합니다.
 */
UCLASS()
class EMPTYCITY_API UECWorkbenchWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	UECWorkbenchWidget(const FObjectInitializer& ObjectInitializer);
	virtual void OnWidgetOpened_Implementation() override;

private:
	/** 추후 모드별 제작 목록을 전달할 때 사용할 공통 슬롯 갱신 함수입니다. */
	void RefreshWorkbenchScrollBox(const TArray<FInventoryEntry>& Entries);
	void RefreshWeaponScrollBox();
	void RefreshWorkbenchContent();
	void RefreshSelectionVisuals();
	void SetWorkbenchMode(EWorkbenchMode NewMode);
	void ClearSelectedItem();
	int32 FindHoveredSlotIndex() const;
	void ApplyWeaponDataToSlot(
		UECInventorySlotWidget* SlotWidget,
		FName WeaponRowName,
		const FECWeaponTableRow& WeaponData,
		bool bIsUnlocked) const;
	void ShowWeaponDetails(FName WeaponRowName);
	void ShowPlaceholderDetails();
	const FECWeaponTableRow* FindWeaponData(FName WeaponRowName) const;
	bool IsWeaponUnlocked(FName WeaponRowName) const;

	void HandleItemSlotClicked(UECInventoryItemInstance* ItemInstance);
	void HandleItemSlotRightClicked(UECInventoryItemInstance* ItemInstance);

	UFUNCTION()
	void HandleStatTabClicked();

	UFUNCTION()
	void HandleToolTabClicked();

	UFUNCTION()
	void HandleWeaponTabClicked();

	UFUNCTION()
	void HandleConfirmActionClicked();

	const TCHAR* GetCurrentModeLogName() const;

	UPROPERTY()
	TObjectPtr<UECInventoryItemInstance> SelectedItemInstance;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> LoadedWeaponDataTable;

	TArray<FName> DisplayedWeaponRowNames;

	FName SelectedWeaponRowName = NAME_None;
	int32 SelectedSlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess = "true")) // 블루프린트 ConfirmAction 사운드 재생 구분
	EWorkbenchMode CurrentWorkbenchMode = EWorkbenchMode::Stat;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry,	const FPointerEvent& InMouseEvent) override;

	/** 현재 탭에 맞는 강화/제작 목록을 표시할 공통 ScrollBox입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox_Inventory;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_StatTab;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_ToolTab;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_WeaponTab;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_WorkbenchModeLabel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ActionValueLabel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_ConfirmAction;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ConfirmAction;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_ItemDetail;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UECItemDetailWidget> ItemDetailWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Workbench")
	TSubclassOf<UECInventorySlotWidget> SlotWidgetClass;

	/** 아직 데이터가 없는 Stat/Tool 탭에 표시할 빈 슬롯 개수입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Workbench", meta = (ClampMin = "0"))
	int32 PlaceholderSlotCount = 5;

	/** Weapon 탭에서 직접 열 DT_Weapon입니다. 필요하면 Workbench BP에서 교체할 수 있습니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Workbench")
	TSoftObjectPtr<UDataTable> WeaponDataTable;
};
