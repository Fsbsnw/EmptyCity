// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/Workbench/ECWorkbenchWidget.h"

#include "Components/Button.h"
#include "Components/Widget.h"
#include "Components/WidgetSwitcher.h"
#include "UI/Widget/Workbench/ECWorkbenchStatUpgradeWidget.h"
#include "UI/Widget/Workbench/ECWorkbenchToolUpgradeWidget.h"
#include "UI/Widget/Workbench/ECWorkbenchWeaponUpgradeWidget.h"

void UECWorkbenchWidget::OnWidgetOpened_Implementation()
{
	Super::OnWidgetOpened_Implementation();
	SetWorkbenchMode(EWorkbenchMode::Stat);
}

void UECWorkbenchWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_StatTab)
	{
		Button_StatTab->OnClicked.RemoveDynamic(this, &ThisClass::HandleStatTabClicked);
		Button_StatTab->OnClicked.AddDynamic(this, &ThisClass::HandleStatTabClicked);
	}

	if (Button_ToolTab)
	{
		Button_ToolTab->OnClicked.RemoveDynamic(this, &ThisClass::HandleToolTabClicked);
		Button_ToolTab->OnClicked.AddDynamic(this, &ThisClass::HandleToolTabClicked);
	}

	if (Button_WeaponTab)
	{
		Button_WeaponTab->OnClicked.RemoveDynamic(this, &ThisClass::HandleWeaponTabClicked);
		Button_WeaponTab->OnClicked.AddDynamic(this, &ThisClass::HandleWeaponTabClicked);
	}

	if (Button_Axe)
	{
		Button_Axe->OnClicked.RemoveDynamic(this, &ThisClass::HandleAxeClicked);
		Button_Axe->OnClicked.AddDynamic(this, &ThisClass::HandleAxeClicked);
	}

	if (Button_Pickaxe)
	{
		Button_Pickaxe->OnClicked.RemoveDynamic(this, &ThisClass::HandlePickaxeClicked);
		Button_Pickaxe->OnClicked.AddDynamic(this, &ThisClass::HandlePickaxeClicked);
	}
}

void UECWorkbenchWidget::RefreshStatUpgradeWidgets()
{
	if (StatUpgradeWidget_MaxHealth)
	{
		StatUpgradeWidget_MaxHealth->SetUpgradeType(EECWorkbenchUpgradeType::MaxHealth);
		StatUpgradeWidget_MaxHealth->RefreshStatPreview();
	}

	if (StatUpgradeWidget_MaxStamina)
	{
		StatUpgradeWidget_MaxStamina->SetUpgradeType(EECWorkbenchUpgradeType::MaxStamina);
		StatUpgradeWidget_MaxStamina->RefreshStatPreview();
	}

	if (StatUpgradeWidget_MaxInventoryWeight)
	{
		StatUpgradeWidget_MaxInventoryWeight->SetUpgradeType(EECWorkbenchUpgradeType::MaxInventoryWeight);
		StatUpgradeWidget_MaxInventoryWeight->RefreshStatPreview();
	}
}

void UECWorkbenchWidget::SetWorkbenchMode(EWorkbenchMode NewMode)
{
	CurrentWorkbenchMode = NewMode;

	if (WidgetSwitcher_ItemDetail)
	{
		const int32 TargetIndex = ResolveDetailPageIndex();
		if (TargetIndex < WidgetSwitcher_ItemDetail->GetNumWidgets())
		{
			WidgetSwitcher_ItemDetail->SetActiveWidgetIndex(TargetIndex);

			if (UWidget* ActiveWidget = WidgetSwitcher_ItemDetail->GetActiveWidget())
			{
				// Stat의 Overlay_Status와 도구 상세는 화면 전체를 덮지만,
				// 자기 자신만 입력에서 제외해 내부 강화 버튼은 계속 사용할 수 있게 합니다.
				switch (CurrentWorkbenchMode)
				{
				case EWorkbenchMode::Stat:
				case EWorkbenchMode::Tool:
				case EWorkbenchMode::Weapon:
					ActiveWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
					break;
				default:
					ActiveWidget->SetVisibility(ESlateVisibility::Visible);
					break;
				}
			}
		}
		else
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("WidgetSwitcher_ItemDetail에 %d번 페이지가 없습니다. 현재 페이지 수: %d"),
				TargetIndex,
				WidgetSwitcher_ItemDetail->GetNumWidgets());
		}
	}

	if (Button_StatTab)
	{
		Button_StatTab->SetIsEnabled(CurrentWorkbenchMode != EWorkbenchMode::Stat);
	}

	if (Button_ToolTab)
	{
		Button_ToolTab->SetIsEnabled(CurrentWorkbenchMode != EWorkbenchMode::Tool);
	}

	if (Button_WeaponTab)
	{
		Button_WeaponTab->SetIsEnabled(CurrentWorkbenchMode != EWorkbenchMode::Weapon);
	}

	RefreshToolTabState();

	if (CurrentWorkbenchMode == EWorkbenchMode::Stat)
	{
		RefreshStatUpgradeWidgets();
	}
	else if (CurrentWorkbenchMode == EWorkbenchMode::Tool)
	{
		if (UECWorkbenchToolUpgradeWidget* ToolDetailWidget = ResolveToolDetailWidget())
		{
			ToolDetailWidget->SelectTool(SelectedTool);
		}
	}
	else if (CurrentWorkbenchMode == EWorkbenchMode::Weapon)
	{
		if (UECWorkbenchWeaponUpgradeWidget* WeaponDetailWidget = ResolveWeaponDetailWidget())
		{
			WeaponDetailWidget->RefreshWeaponPreview();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Workbench tab selected: %s"), GetCurrentModeLogName());
}

void UECWorkbenchWidget::HandleStatTabClicked()
{
	SetWorkbenchMode(EWorkbenchMode::Stat);
}

void UECWorkbenchWidget::HandleToolTabClicked()
{
	SetWorkbenchMode(EWorkbenchMode::Tool);
}

void UECWorkbenchWidget::HandleWeaponTabClicked()
{
	SetWorkbenchMode(EWorkbenchMode::Weapon);
}

void UECWorkbenchWidget::HandleAxeClicked()
{
	SetSelectedTool(EECWorkbenchUpgradeType::Axe);
}

void UECWorkbenchWidget::HandlePickaxeClicked()
{
	SetSelectedTool(EECWorkbenchUpgradeType::Pickaxe);
}

void UECWorkbenchWidget::SetSelectedTool(EECWorkbenchUpgradeType NewTool)
{
	SelectedTool = NewTool == EECWorkbenchUpgradeType::Pickaxe
		? EECWorkbenchUpgradeType::Pickaxe
		: EECWorkbenchUpgradeType::Axe;

	RefreshToolTabState();
	if (CurrentWorkbenchMode == EWorkbenchMode::Tool)
	{
		SetWorkbenchMode(EWorkbenchMode::Tool);
	}
}

void UECWorkbenchWidget::RefreshToolTabState()
{
	const bool bShowToolTabs = CurrentWorkbenchMode == EWorkbenchMode::Tool;
	const ESlateVisibility PanelVisibility = bShowToolTabs
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed;
	const ESlateVisibility ButtonVisibility = bShowToolTabs
		? ESlateVisibility::Visible
		: ESlateVisibility::Collapsed;

	if (Panel_ToolTabs)
	{
		Panel_ToolTabs->SetVisibility(PanelVisibility);
	}
	if (Button_Axe)
	{
		Button_Axe->SetVisibility(ButtonVisibility);
		Button_Axe->SetIsEnabled(SelectedTool != EECWorkbenchUpgradeType::Axe);
	}
	if (Button_Pickaxe)
	{
		Button_Pickaxe->SetVisibility(ButtonVisibility);
		Button_Pickaxe->SetIsEnabled(SelectedTool != EECWorkbenchUpgradeType::Pickaxe);
	}
}

UECWorkbenchToolUpgradeWidget* UECWorkbenchWidget::ResolveToolDetailWidget() const
{
	if (!WidgetSwitcher_ItemDetail || CurrentWorkbenchMode != EWorkbenchMode::Tool)
	{
		return nullptr;
	}

	const int32 ToolPageIndex = ResolveDetailPageIndex();
	if (ToolPageIndex >= WidgetSwitcher_ItemDetail->GetNumWidgets())
	{
		return nullptr;
	}

	return Cast<UECWorkbenchToolUpgradeWidget>(
		WidgetSwitcher_ItemDetail->GetWidgetAtIndex(ToolPageIndex));
}

UECWorkbenchWeaponUpgradeWidget* UECWorkbenchWidget::ResolveWeaponDetailWidget() const
{
	if (!WidgetSwitcher_ItemDetail || CurrentWorkbenchMode != EWorkbenchMode::Weapon)
	{
		return nullptr;
	}

	const int32 WeaponPageIndex = ResolveDetailPageIndex();
	if (WeaponPageIndex >= WidgetSwitcher_ItemDetail->GetNumWidgets())
	{
		return nullptr;
	}

	return Cast<UECWorkbenchWeaponUpgradeWidget>(
		WidgetSwitcher_ItemDetail->GetWidgetAtIndex(WeaponPageIndex));
}

int32 UECWorkbenchWidget::ResolveDetailPageIndex() const
{
	switch (CurrentWorkbenchMode)
	{
	case EWorkbenchMode::Stat:
		return 0;
	case EWorkbenchMode::Tool:
		return SelectedTool == EECWorkbenchUpgradeType::Pickaxe ? 2 : 1;
	case EWorkbenchMode::Weapon:
		return 3;
	}

	return 0;
}

const TCHAR* UECWorkbenchWidget::GetCurrentModeLogName() const
{
	switch (CurrentWorkbenchMode)
	{
	case EWorkbenchMode::Stat:
		return TEXT("Stat");
	case EWorkbenchMode::Tool:
		return TEXT("Tool");
	case EWorkbenchMode::Weapon:
		return TEXT("Weapon");
	}

	return TEXT("Unknown");
}
