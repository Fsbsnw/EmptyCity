// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECBedMenuWidget.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECBedMenuWidget : public UECUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void OnSleepYesButtonClicked();

	UFUNCTION()
	void OnSleepNoButtonClicked();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Sleep_Yes;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Sleep_No;
};
