// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ECMainLayoutWidget.generated.h"

class UOverlay;
class UCanvasPanel;
/**
 * 플레이어의 화면에 표시될 기본 레이아웃 위젯입니다.
 */
UCLASS()
class EMPTYCITY_API UECMainLayoutWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/** 최상위 캔버스 */
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* MainCanvas;

	/** 1층: 게임 화면에 항상 깔려있는 HUD (체력바, 미니맵 등) */
	UPROPERTY(meta = (BindWidget))
	UOverlay* Layer_HUD;

	/** 2층: 그 위를 덮는 윈도우 창들 (인벤토리, 맵 등) */
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* Layer_Window;
};
