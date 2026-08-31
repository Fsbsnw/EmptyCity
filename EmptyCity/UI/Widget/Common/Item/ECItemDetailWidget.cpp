// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/Common/Item/ECItemDetailWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/Item/ECItemTableRow.h"
#include "Data/Weapon/ECWeaponTableRow.h"
#include "Engine/GameInstance.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Subsystem/ECItemDataSubsystem.h"
#include "Subsystem/ECProgressionSubsystem.h"

void UECItemDetailWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ClearItem();
}

void UECItemDetailWidget::SetItem(UECInventoryItemInstance* ItemInstance, int32 StackCount, int32 Price)
{
	ClearWeaponData();

	if (!IsValid(ItemInstance))
	{
		ClearItem();
		return;
	}

	const FECItemTableRow* ItemData = UECItemDataSubsystem::Get(this).FindItemData(ItemInstance);
	if (!ItemData)
	{
		ClearItem();
		return;
	}

	CachedItemInstance = ItemInstance;

	bool bIsUnlocked = true;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UECProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UECProgressionSubsystem>())
		{
			bIsUnlocked = ProgressionSubsystem->IsItemUnlocked(ItemInstance->ItemDef);
		}
	}

	if (Image_ItemIcon)
	{
		const TSoftObjectPtr<UTexture2D>& ItemIcon = bIsUnlocked ? ItemData->Icon : ItemData->LockedIcon;
		if (!ItemIcon.IsNull())
		{
			Image_ItemIcon->SetBrushFromSoftTexture(ItemIcon, false);
		}
		else
		{
			Image_ItemIcon->SetBrush(FSlateBrush());
		}

		Image_ItemIcon->SetOpacity(bIsUnlocked ? 1.0f : 0.5f);
	}

	if (Text_ItemName)
	{
		Text_ItemName->SetText(ItemData->DisplayName);
	}

	if (Text_ItemDescription)
	{
		const FText& Description = bIsUnlocked || ItemData->LockedDescription.IsEmpty()
			? ItemData->Description
			: ItemData->LockedDescription;
		Text_ItemDescription->SetText(Description);
	}

	if (Text_ItemPrice)
	{
		Text_ItemPrice->SetText(FText::AsNumber(FMath::Max(0, Price)));
	}

	if (Text_ItemWeight)
	{
		FNumberFormattingOptions WeightFormatting;
		WeightFormatting.MinimumFractionalDigits = 0;
		WeightFormatting.MaximumFractionalDigits = 2;
		Text_ItemWeight->SetText(FText::AsNumber(ItemData->Weight, &WeightFormatting));
	}

	if (Text_ItemCount)
	{
		Text_ItemCount->SetText(FText::AsNumber(FMath::Max(0, StackCount)));
	}

	SetVisibility(ESlateVisibility::Visible);
}

void UECItemDetailWidget::SetWeaponData(const FECWeaponTableRow* WeaponData, bool bIsUnlocked)
{
	ClearWeaponData();

	if (!WeaponData)
	{
		return;
	}

	FNumberFormattingOptions NumberFormatting;
	NumberFormatting.MinimumFractionalDigits = 0;
	NumberFormatting.MaximumFractionalDigits = 2;

	auto SetNumberText = [&NumberFormatting](UTextBlock* TextBlock, float Value)
	{
		if (TextBlock)
		{
			TextBlock->SetText(FText::AsNumber(Value, &NumberFormatting));
			TextBlock->SetVisibility(ESlateVisibility::Visible);
		}
	};

	if (Text_ItemUnlockState)
	{
		Text_ItemUnlockState->SetText(FText::FromString(bIsUnlocked ? TEXT("해금") : TEXT("미해금")));
		Text_ItemUnlockState->SetVisibility(ESlateVisibility::Visible);
	}

	// Weapon 탭에서는 DT_Weapon의 무게를 표시합니다.
	SetNumberText(Text_ItemWeight, WeaponData->Weight);
	SetNumberText(Text_WeaponAttackPower, WeaponData->AttackPower);
	SetNumberText(Text_WeaponAttackSpeed, WeaponData->AttackSpeedMultiplier);
	SetNumberText(Text_WeaponLightStaminaCost, WeaponData->LightAttackStaminaCost);
	SetNumberText(Text_WeaponHeavyStaminaCost, WeaponData->HeavyAttackStaminaCost);
}

void UECItemDetailWidget::ClearItem()
{
	CachedItemInstance = nullptr;
	ClearWeaponData();

	if (Image_ItemIcon)
	{
		Image_ItemIcon->SetBrush(FSlateBrush());
		Image_ItemIcon->SetOpacity(1.0f);
	}

	if (Text_ItemName)
	{
		Text_ItemName->SetText(FText::GetEmpty());
	}

	if (Text_ItemDescription)
	{
		Text_ItemDescription->SetText(FText::GetEmpty());
	}

	if (Text_ItemPrice)
	{
		Text_ItemPrice->SetText(FText::GetEmpty());
	}

	if (Text_ItemWeight)
	{
		Text_ItemWeight->SetText(FText::GetEmpty());
	}

	if (Text_ItemCount)
	{
		Text_ItemCount->SetText(FText::GetEmpty());
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

void UECItemDetailWidget::ClearWeaponData()
{
	auto ClearTextBlock = [](UTextBlock* TextBlock)
	{
		if (TextBlock)
		{
			TextBlock->SetText(FText::GetEmpty());
			TextBlock->SetVisibility(ESlateVisibility::Collapsed);
		}
	};

	ClearTextBlock(Text_ItemUnlockState);
	ClearTextBlock(Text_WeaponAttackPower);
	ClearTextBlock(Text_WeaponAttackSpeed);
	ClearTextBlock(Text_WeaponLightStaminaCost);
	ClearTextBlock(Text_WeaponHeavyStaminaCost);
}
