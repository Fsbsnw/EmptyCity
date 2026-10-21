// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Bed/ECBedMenuWidget.h"

#include "Components/Button.h"
#include "UI/ViewModel/BedMenuViewModel.h"

void UECBedMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Sleep_Yes)
	{
		Button_Sleep_Yes->OnClicked.AddUniqueDynamic(this, &ThisClass::OnSleepYesButtonClicked);
	}

	if (Button_Sleep_No)
	{
		Button_Sleep_No->OnClicked.AddUniqueDynamic(this, &ThisClass::OnSleepNoButtonClicked);
	}
}

void UECBedMenuWidget::NativeDestruct()
{
	if (Button_Sleep_Yes)
	{
		Button_Sleep_Yes->OnClicked.RemoveDynamic(this, &ThisClass::OnSleepYesButtonClicked);
	}

	if (Button_Sleep_No)
	{
		Button_Sleep_No->OnClicked.RemoveDynamic(this, &ThisClass::OnSleepNoButtonClicked);
	}

	Super::NativeDestruct();
}

void UECBedMenuWidget::OnSleepYesButtonClicked()
{
	if (UBedMenuViewModel* BedMenuViewModel = GetViewModel<UBedMenuViewModel>())
	{
		BedMenuViewModel->RequestSleepSequence();
	}
}

void UECBedMenuWidget::OnSleepNoButtonClicked()
{
	if (UBedMenuViewModel* BedMenuViewModel = GetViewModel<UBedMenuViewModel>())
	{
		BedMenuViewModel->RequestCloseMenu();
	}
}
