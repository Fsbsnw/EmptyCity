// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	
protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_MapNodeName;
	
public:
	// 외부에서 이름을 받아 텍스트를 갱신하는 함수
	void SetMapNodeName(const FText& InMapName);
};
