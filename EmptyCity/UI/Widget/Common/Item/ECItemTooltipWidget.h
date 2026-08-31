// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECItemTooltipWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECItemTooltipWidget : public UECUserWidget
{
	GENERATED_BODY()
public:
	/** 툴팁 정보를 업데이트 합니다. */
	void UpdateTooltipText(const FText& InName, const FText& InDescription);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemDescriptionText;
};
