// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Inventory/ECItemTooltipWidget.h"

#include "Components/TextBlock.h"

void UECItemTooltipWidget::UpdateTooltipText(const FText& InName, const FText& InDescription)
{
	if (ItemNameText)
	{
		ItemNameText->SetText(InName);
	}
    
	if (ItemDescriptionText)
	{
		ItemDescriptionText->SetText(InDescription);
	}
}
