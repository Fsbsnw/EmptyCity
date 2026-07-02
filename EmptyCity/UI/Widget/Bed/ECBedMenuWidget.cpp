// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Bed/ECBedMenuWidget.h"

#include "Components/Button.h"
#include "UI/ViewModel/BedMenuViewModel.h"

void UECBedMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Sleep)
	{
		Button_Sleep->OnClicked.AddDynamic(this, &ThisClass::OnSleepButtonClicked);
	}
}

void UECBedMenuWidget::OnSleepButtonClicked()
{
	if (UBedMenuViewModel* BedMenuViewModel = GetViewModel<UBedMenuViewModel>())
	{
		BedMenuViewModel->RequestSleepSequence();
	}
}