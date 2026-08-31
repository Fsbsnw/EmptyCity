// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECMapNodeInfoWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECMapNodeInfoWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	void SetMapName(FText MapName) const { Text_MapName->SetText(MapName); };
	void SetMapDescription(FText MapDescription) const { Text_MapDescription->SetText(MapDescription); };
	
protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* Text_MapName;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* Text_MapDescription;
};