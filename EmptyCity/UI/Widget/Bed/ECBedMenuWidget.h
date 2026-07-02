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
	
	UFUNCTION()
	void OnSleepButtonClicked();

private:
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Sleep;
};
