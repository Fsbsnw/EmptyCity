// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Workbench/ECWorkbenchWidget.h"

#include "ECGameplayTags.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Data/Weapon/ECWeaponTableRow.h"
#include "Engine/DataTable.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "Subsystem/ECProgressionSubsystem.h"
#include "UI/Widget/Inventory/ECInventorySlotWidget.h"
#include "UI/Widget/Common/Item/ECItemDetailWidget.h"

UECWorkbenchWidget::UECWorkbenchWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WeaponDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(
		TEXT("/Game/_EmptyCity/Blueprints/Data/Weapon/DT_Weapon.DT_Weapon")));
}

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

	if (Button_ConfirmAction)
	{
		Button_ConfirmAction->OnClicked.RemoveDynamic(this, &ThisClass::HandleConfirmActionClicked);
		Button_ConfirmAction->OnClicked.AddDynamic(this, &ThisClass::HandleConfirmActionClicked);
	}
}

void UECWorkbenchWidget::NativeDestruct()
{
	ClearSelectedItem();
	DisplayedWeaponRowNames.Reset();
	LoadedWeaponDataTable = nullptr;
	
	Super::NativeDestruct();
}

FReply UECWorkbenchWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		ClearSelectedItem();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UECWorkbenchWidget::RefreshWorkbenchScrollBox(const TArray<FInventoryEntry>& Entries)
{
	if (!ScrollBox_Inventory || !SlotWidgetClass)
	{
		return;
	}

	while (ScrollBox_Inventory->GetChildrenCount() < Entries.Num())
	{
		UECInventorySlotWidget* SlotWidget = CreateWidget<UECInventorySlotWidget>(this, SlotWidgetClass);
		if (!SlotWidget)
		{
			break;
		}

		SlotWidget->OnClicked.BindUObject(this, &ThisClass::HandleItemSlotClicked);
		SlotWidget->OnRightClicked.BindUObject(this, &ThisClass::HandleItemSlotRightClicked);
		ScrollBox_Inventory->AddChild(SlotWidget);
	}

	while (ScrollBox_Inventory->GetChildrenCount() > Entries.Num())
	{
		ScrollBox_Inventory->RemoveChildAt(ScrollBox_Inventory->GetChildrenCount() - 1);
	}

	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		if (UECInventorySlotWidget* SlotWidget = Cast<UECInventorySlotWidget>(ScrollBox_Inventory->GetChildAt(Index)))
		{
			SlotWidget->RefreshSlotUI(Entries[Index].Instance, Entries[Index].StackCount);
			SlotWidget->SetSelected(Index == SelectedSlotIndex);
		}
	}
}

void UECWorkbenchWidget::RefreshWeaponScrollBox()
{
	if (!ScrollBox_Inventory || !SlotWidgetClass)
	{
		return;
	}

	if (!IsValid(LoadedWeaponDataTable))
	{
		LoadedWeaponDataTable = WeaponDataTable.LoadSynchronous();
	}

	if (!IsValid(LoadedWeaponDataTable) || LoadedWeaponDataTable->GetRowStruct() != FECWeaponTableRow::StaticStruct())
	{
		UE_LOG(LogTemp, Error, TEXT("Workbench에서 유효한 DT_Weapon을 불러오지 못했습니다."));
		DisplayedWeaponRowNames.Reset();
		ScrollBox_Inventory->ClearChildren();
		return;
	}

	DisplayedWeaponRowNames = LoadedWeaponDataTable->GetRowNames();
	DisplayedWeaponRowNames.Sort([](const FName& A, const FName& B)
	{
		return A.LexicalLess(B);
	});

	while (ScrollBox_Inventory->GetChildrenCount() < DisplayedWeaponRowNames.Num())
	{
		UECInventorySlotWidget* SlotWidget = CreateWidget<UECInventorySlotWidget>(this, SlotWidgetClass);
		if (!SlotWidget)
		{
			break;
		}

		ScrollBox_Inventory->AddChild(SlotWidget);
	}

	while (ScrollBox_Inventory->GetChildrenCount() > DisplayedWeaponRowNames.Num())
	{
		ScrollBox_Inventory->RemoveChildAt(ScrollBox_Inventory->GetChildrenCount() - 1);
	}

	for (int32 Index = 0; Index < DisplayedWeaponRowNames.Num(); ++Index)
	{
		UECInventorySlotWidget* SlotWidget = Cast<UECInventorySlotWidget>(ScrollBox_Inventory->GetChildAt(Index));
		const FECWeaponTableRow* WeaponData = FindWeaponData(DisplayedWeaponRowNames[Index]);
		if (!SlotWidget || !WeaponData)
		{
			continue;
		}

		// Player/TradingPost와 동일하게 슬롯 자체의 클릭 델리게이트에서 상세창을 엽니다.
		SlotWidget->OnClicked.BindUObject(this, &ThisClass::HandleItemSlotClicked);
		SlotWidget->OnRightClicked.BindUObject(this, &ThisClass::HandleItemSlotRightClicked);

		ApplyWeaponDataToSlot(
			SlotWidget,
			DisplayedWeaponRowNames[Index],
			*WeaponData,
			IsWeaponUnlocked(DisplayedWeaponRowNames[Index]));
		SlotWidget->SetSelected(
			!SelectedWeaponRowName.IsNone() &&
			DisplayedWeaponRowNames[Index] == SelectedWeaponRowName);
	}

	UE_LOG(LogTemp,	Log, TEXT("Workbench weapon content refreshed directly from DT_Weapon: %d rows"), DisplayedWeaponRowNames.Num());
}

void UECWorkbenchWidget::ApplyWeaponDataToSlot(UECInventorySlotWidget* SlotWidget, FName WeaponRowName,
	const FECWeaponTableRow& WeaponData, bool bIsUnlocked) const
{
	if (!SlotWidget)
	{
		return;
	}

	SlotWidget->SetEmptySlot();
	SlotWidget->SetRenderOpacity(bIsUnlocked ? 1.0f : 0.5f);

	if (UImage* ItemIcon = Cast<UImage>(SlotWidget->GetWidgetFromName(TEXT("Image_ItemIcon"))))
	{
		// 현재 DT_Weapon에는 아이콘 열이 없으므로 슬롯의 텍스트 정보만 사용합니다.
		ItemIcon->SetBrush(FSlateBrush());
		ItemIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	FText DisplayName = WeaponData.KoreanName;
	if (DisplayName.IsEmpty())
	{
		DisplayName = WeaponData.EnglishName.IsEmpty()? FText::FromName(WeaponRowName) : WeaponData.EnglishName;
	}

	if (UTextBlock* ItemName = Cast<UTextBlock>(SlotWidget->GetWidgetFromName(TEXT("Text_ItemName"))))
	{
		ItemName->SetText(DisplayName);
		ItemName->SetVisibility(ESlateVisibility::Visible);
	}

	if (UTextBlock* ItemDescription = Cast<UTextBlock>(SlotWidget->GetWidgetFromName(TEXT("Text_ItemDescription"))))
	{
		ItemDescription->SetText(WeaponData.EnglishName);
		ItemDescription->SetVisibility(WeaponData.EnglishName.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	if (UTextBlock* ItemWeight = Cast<UTextBlock>(SlotWidget->GetWidgetFromName(TEXT("Text_ItemWeight"))))
	{
		FNumberFormattingOptions NumberFormatting;
		NumberFormatting.MinimumFractionalDigits = 0;
		NumberFormatting.MaximumFractionalDigits = 2;
		ItemWeight->SetText(FText::AsNumber(WeaponData.Weight, &NumberFormatting));
		ItemWeight->SetVisibility(ESlateVisibility::Visible);
	}
}

void UECWorkbenchWidget::ShowWeaponDetails(FName WeaponRowName)
{
	const FECWeaponTableRow* WeaponData = FindWeaponData(WeaponRowName);
	if (!WeaponData)
	{
		ClearSelectedItem();
		return;
	}

	SelectedItemInstance = nullptr;
	SelectedWeaponRowName = WeaponRowName;

	if (ItemDetailWidget)
	{
		ItemDetailWidget->ClearItem();
		ItemDetailWidget->SetWeaponData(WeaponData, IsWeaponUnlocked(WeaponRowName));

		FText DisplayName = WeaponData->KoreanName;
		if (DisplayName.IsEmpty())
		{
			DisplayName = WeaponData->EnglishName.IsEmpty()
				? FText::FromName(WeaponRowName)
				: WeaponData->EnglishName;
		}

		if (UTextBlock* ItemName =
			Cast<UTextBlock>(ItemDetailWidget->GetWidgetFromName(TEXT("Text_ItemName"))))
		{
			ItemName->SetText(DisplayName);
		}

		if (UTextBlock* ItemDescription =
			Cast<UTextBlock>(ItemDetailWidget->GetWidgetFromName(TEXT("Text_ItemDescription"))))
		{
			ItemDescription->SetText(WeaponData->EnglishName);
		}

		if (UImage* ItemIcon =
			Cast<UImage>(ItemDetailWidget->GetWidgetFromName(TEXT("Image_ItemIcon"))))
		{
			ItemIcon->SetBrush(FSlateBrush());
			ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		}

		ItemDetailWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (Overlay_ItemDetail)
	{
		Overlay_ItemDetail->SetVisibility(ESlateVisibility::Visible);
	}

	UE_LOG(LogTemp, Log, TEXT("Workbench weapon selected: %s"), *WeaponRowName.ToString());
}

void UECWorkbenchWidget::ShowPlaceholderDetails()
{
	SelectedItemInstance = nullptr;
	SelectedWeaponRowName = NAME_None;

	const FText ModeName = CurrentWorkbenchMode == EWorkbenchMode::Stat
		? FText::FromString(TEXT("스탯"))
		: FText::FromString(TEXT("도구"));

	if (ItemDetailWidget)
	{
		ItemDetailWidget->ClearItem();

		if (UTextBlock* ItemName =
			Cast<UTextBlock>(ItemDetailWidget->GetWidgetFromName(TEXT("Text_ItemName"))))
		{
			ItemName->SetText(ModeName);
		}

		if (UTextBlock* ItemDescription =
			Cast<UTextBlock>(ItemDetailWidget->GetWidgetFromName(TEXT("Text_ItemDescription"))))
		{
			ItemDescription->SetText(FText::FromString(TEXT("아직 구현되지 않은 항목입니다.")));
		}

		ItemDetailWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (Overlay_ItemDetail)
	{
		Overlay_ItemDetail->SetVisibility(ESlateVisibility::Visible);
	}

	UE_LOG(LogTemp, Log, TEXT("Workbench placeholder selected. Mode: %s"), GetCurrentModeLogName());
}

void UECWorkbenchWidget::RefreshWorkbenchContent()
{
	if (CurrentWorkbenchMode == EWorkbenchMode::Weapon)
	{
		RefreshWeaponScrollBox();
		return;
	}

	DisplayedWeaponRowNames.Reset();
	TArray<FInventoryEntry> PlaceholderEntries;
	PlaceholderEntries.SetNum(PlaceholderSlotCount);
	RefreshWorkbenchScrollBox(PlaceholderEntries);
	UE_LOG(LogTemp, Log, TEXT("Workbench content refresh requested. Mode: %s (Not Implemented)"), GetCurrentModeLogName());
}

void UECWorkbenchWidget::RefreshSelectionVisuals()
{
	if (!ScrollBox_Inventory)
	{
		return;
	}

	for (int32 Index = 0; Index < ScrollBox_Inventory->GetChildrenCount(); ++Index)
	{
		UECInventorySlotWidget* SlotWidget =
			Cast<UECInventorySlotWidget>(ScrollBox_Inventory->GetChildAt(Index));
		if (!SlotWidget)
		{
			continue;
		}

		const bool bIsSelected = CurrentWorkbenchMode == EWorkbenchMode::Weapon
			? !SelectedWeaponRowName.IsNone() &&
			  DisplayedWeaponRowNames.IsValidIndex(Index) &&
			  DisplayedWeaponRowNames[Index] == SelectedWeaponRowName
			: Index == SelectedSlotIndex;
		SlotWidget->SetSelected(bIsSelected);
	}
}

int32 UECWorkbenchWidget::FindHoveredSlotIndex() const
{
	if (!ScrollBox_Inventory)
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index < ScrollBox_Inventory->GetChildrenCount(); ++Index)
	{
		if (const UWidget* SlotWidget = ScrollBox_Inventory->GetChildAt(Index);
			SlotWidget && SlotWidget->IsHovered())
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

const FECWeaponTableRow* UECWorkbenchWidget::FindWeaponData(FName WeaponRowName) const
{
	if (!IsValid(LoadedWeaponDataTable) || WeaponRowName.IsNone())
	{
		return nullptr;
	}

	return LoadedWeaponDataTable->FindRow<FECWeaponTableRow>(
		WeaponRowName,
		TEXT("UECWorkbenchWidget::FindWeaponData"),
		false);
}

bool UECWorkbenchWidget::IsWeaponUnlocked(FName WeaponRowName) const
{
	if (!FindWeaponData(WeaponRowName))
	{
		return false;
	}

	FGameplayTag WeaponUnlockTag;
	if (WeaponRowName == TEXT("WPN01"))
	{
		WeaponUnlockTag = ECGameplayTags::Weapon_WoodenClub;
	}
	else if (WeaponRowName == TEXT("WPN02"))
	{
		WeaponUnlockTag = ECGameplayTags::Weapon_PipeClub;
	}
	else if (WeaponRowName == TEXT("WPN03"))
	{
		WeaponUnlockTag = ECGameplayTags::Weapon_SteelClub;
	}
	else
	{
		// 아직 해금 태그 연결 규칙이 없는 행은 기본 해금 처리합니다.
		return true;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UECProgressionSubsystem* ProgressionSubsystem =
			GameInstance->GetSubsystem<UECProgressionSubsystem>())
		{
			return ProgressionSubsystem->IsContentUnlocked(WeaponUnlockTag);
		}
	}

	return false;
}

void UECWorkbenchWidget::SetWorkbenchMode(EWorkbenchMode NewMode)
{
	CurrentWorkbenchMode = NewMode;
	ClearSelectedItem();

	FText ModeLabel;
	FText ActionValueLabel;
	FText ConfirmActionText;

	switch (CurrentWorkbenchMode)
	{
	case EWorkbenchMode::Stat:
		ModeLabel = FText::FromString(TEXT("스탯"));
		ActionValueLabel = FText::FromString(TEXT("강화 비용"));
		ConfirmActionText = FText::FromString(TEXT("강화"));
		break;

	case EWorkbenchMode::Tool:
		ModeLabel = FText::FromString(TEXT("도구"));
		ActionValueLabel = FText::FromString(TEXT("제작 비용"));
		ConfirmActionText = FText::FromString(TEXT("제작"));
		break;

	case EWorkbenchMode::Weapon:
		ModeLabel = FText::FromString(TEXT("무기"));
		ActionValueLabel = FText::FromString(TEXT("제작 비용"));
		ConfirmActionText = FText::FromString(TEXT("제작"));
		break;
	}

	if (Text_WorkbenchModeLabel)
	{
		Text_WorkbenchModeLabel->SetText(ModeLabel);
	}

	if (Text_ActionValueLabel)
	{
		Text_ActionValueLabel->SetText(ActionValueLabel);
	}

	if (Text_ConfirmAction)
	{
		Text_ConfirmAction->SetText(ConfirmActionText);
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

	RefreshWorkbenchContent();

	if (ScrollBox_Inventory)
	{
		ScrollBox_Inventory->ScrollToStart();
	}

	UE_LOG(LogTemp, Log, TEXT("Workbench tab selected: %s"), GetCurrentModeLogName());
}

void UECWorkbenchWidget::ClearSelectedItem()
{
	SelectedItemInstance = nullptr;
	SelectedWeaponRowName = NAME_None;
	SelectedSlotIndex = INDEX_NONE;
	RefreshSelectionVisuals();

	if (Button_ConfirmAction)
	{
		// 실제 데이터가 연결되기 전에도 버튼 클릭 로그를 확인할 수 있게 유지합니다.
		Button_ConfirmAction->SetIsEnabled(true);
	}

	if (ItemDetailWidget)
	{
		ItemDetailWidget->ClearItem();
	}

	if (Overlay_ItemDetail)
	{
		Overlay_ItemDetail->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UECWorkbenchWidget::HandleItemSlotClicked(UECInventoryItemInstance* ItemInstance)
{
	const int32 ClickedSlotIndex = FindHoveredSlotIndex();
	if (ClickedSlotIndex == INDEX_NONE)
	{
		ClearSelectedItem();
		return;
	}

	SelectedSlotIndex = ClickedSlotIndex;

	if (CurrentWorkbenchMode == EWorkbenchMode::Weapon)
	{
		if (DisplayedWeaponRowNames.IsValidIndex(ClickedSlotIndex))
		{
			ShowWeaponDetails(DisplayedWeaponRowNames[ClickedSlotIndex]);
			RefreshSelectionVisuals();
			return;
		}

		ClearSelectedItem();
		return;
	}

	if (!IsValid(ItemInstance))
	{
		ShowPlaceholderDetails();
		RefreshSelectionVisuals();
		return;
	}

	SelectedItemInstance = ItemInstance;

	if (ItemDetailWidget)
	{
		ItemDetailWidget->SetItem(ItemInstance);
	}

	if (Overlay_ItemDetail)
	{
		Overlay_ItemDetail->SetVisibility(ESlateVisibility::Visible);
	}

	RefreshSelectionVisuals();

	UE_LOG(LogTemp,	Log, TEXT("Workbench item selected. Mode: %s, Item: %s"), GetCurrentModeLogName(), *GetNameSafe(ItemInstance));
}

void UECWorkbenchWidget::HandleItemSlotRightClicked(UECInventoryItemInstance* ItemInstance)
{
	UE_LOG(LogTemp,	Log, TEXT("Workbench item right-clicked. Mode: %s, Item: %s (Not Implemented)"), GetCurrentModeLogName(), *GetNameSafe(ItemInstance));
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

void UECWorkbenchWidget::HandleConfirmActionClicked()
{
	const FString SelectedName = CurrentWorkbenchMode == EWorkbenchMode::Weapon
		? SelectedWeaponRowName.ToString()
		: GetNameSafe(SelectedItemInstance);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Workbench action requested. Mode: %s, Item: %s (Not Implemented)"),
		GetCurrentModeLogName(),
		*SelectedName);

	RefreshSelectionVisuals();
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
