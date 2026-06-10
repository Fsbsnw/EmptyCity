// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NavigableWidgetInterface.generated.h"

// 위젯 탐색 시 사용되는 입력 방향입니다.
UENUM(BlueprintType)
enum class EWidgetNavDirection : uint8
{
	Up, Down, Left, Right
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNavigableWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief 위젯 내에서 키보드 조작이 필요한 경우에 상속받는 인터페이스입니다.
 */
class EMPTYCITY_API INavigableWidgetInterface
{
	GENERATED_BODY()
public:
	/** 위젯의 방향키 입력을 처리하는 함수입니다. */
	UFUNCTION(BlueprintNativeEvent)
	void ReceiveDirectionInput(EWidgetNavDirection Direction);
	/** 위젯의 확인 입력을 처리하는 함수입니다. */
	UFUNCTION(BlueprintNativeEvent)
	void ReceiveConfirmInput();
};
