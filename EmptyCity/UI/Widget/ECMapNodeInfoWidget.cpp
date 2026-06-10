// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/ECMapNodeInfoWidget.h"

#include "Components/TextBlock.h"

void UECMapNodeInfoWidget::SetMapNodeName(const FText& InMapName)
{
	if (Text_MapNodeName)
	{
		Text_MapNodeName->SetText(InMapName);
	}
}
