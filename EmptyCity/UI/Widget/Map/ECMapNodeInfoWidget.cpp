// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Map/ECMapNodeInfoWidget.h"

void UECMapNodeInfoWidget::SetRegionImage(UTexture2D* RegionImage) const
{
	if (!Image_Region)
	{
		return;
	}

	Image_Region->SetBrushFromTexture(RegionImage);
	Image_Region->SetVisibility(
		RegionImage ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}
