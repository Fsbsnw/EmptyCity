// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECMapNodeInfoWidget.generated.h"

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECMapNodeInfoWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	void SetMapNodeInfoImage(UTexture2D* MapNodeInfoImage) const { Image_MapNodeInfo->SetBrushFromTexture(MapNodeInfoImage); };
	void SetRegionImage(UTexture2D* RegionImage) const;
	
protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* Image_MapNodeInfo;

	/** 지역 대표 이미지입니다. 기존 WBP와 호환되도록 선택적으로 바인딩합니다. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Region;
};
