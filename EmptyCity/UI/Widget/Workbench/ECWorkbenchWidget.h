// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Workbench/ECWorkbenchUpgradeTypes.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECWorkbenchWidget.generated.h"

class UButton;
class UECWorkbenchStatUpgradeWidget;
class UECWorkbenchToolUpgradeWidget;
class UECWorkbenchWeaponUpgradeWidget;
class UWidget;
class UWidgetSwitcher;

UENUM(BlueprintType)
enum class EWorkbenchMode : uint8
{
	Stat,
	Tool,
	Weapon
};

/** 작업대 탭과 탭별 상세 페이지 전환을 관리합니다. */
UCLASS()
class EMPTYCITY_API UECWorkbenchWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	virtual void OnWidgetOpened_Implementation() override;

protected:
	virtual void NativeConstruct() override;

private:
	void RefreshStatUpgradeWidgets();
	void SetWorkbenchMode(EWorkbenchMode NewMode);
	void SetSelectedTool(EECWorkbenchUpgradeType NewTool);
	void RefreshToolTabState();
	int32 ResolveDetailPageIndex() const;
	UECWorkbenchToolUpgradeWidget* ResolveToolDetailWidget() const;
	UECWorkbenchWeaponUpgradeWidget* ResolveWeaponDetailWidget() const;

	UFUNCTION()
	void HandleStatTabClicked();

	UFUNCTION()
	void HandleToolTabClicked();

	UFUNCTION()
	void HandleWeaponTabClicked();

	UFUNCTION()
	void HandleAxeClicked();

	UFUNCTION()
	void HandlePickaxeClicked();

	const TCHAR* GetCurrentModeLogName() const;

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EWorkbenchMode CurrentWorkbenchMode = EWorkbenchMode::Stat;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EECWorkbenchUpgradeType SelectedTool = EECWorkbenchUpgradeType::Axe;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_StatTab;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_ToolTab;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_WeaponTab;

	/** Tool 탭 안에서 도끼 상세를 선택하는 하위 탭 버튼입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Axe;

	/** Tool 탭 안에서 곡괭이 상세를 선택하는 하위 탭 버튼입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Pickaxe;

	/** Axe/Pickaxe 하위 탭 영역입니다. Tool 탭에서만 표시됩니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> Panel_ToolTabs;

	/** 0: 스탯, 1: Axe 상세, 2: Pickaxe 상세, 3: 무기 상세 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher_ItemDetail;

	/** 스탯 탭에서 항상 표시되는 최대 체력 카드입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UECWorkbenchStatUpgradeWidget> StatUpgradeWidget_MaxHealth;

	/** 스탯 탭에서 항상 표시되는 최대 스태미나 카드입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UECWorkbenchStatUpgradeWidget> StatUpgradeWidget_MaxStamina;

	/** 스탯 탭에서 항상 표시되는 최대 가방 무게 카드입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UECWorkbenchStatUpgradeWidget> StatUpgradeWidget_MaxInventoryWeight;
};
