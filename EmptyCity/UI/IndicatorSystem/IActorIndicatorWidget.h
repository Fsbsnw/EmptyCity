// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"

#include "IActorIndicatorWidget.generated.h"

class AActor;
class UIndicatorDescriptor;

/**
 * Indicator Widget이 IndicatorDescriptor와 데이터를 연결하거나 해제할 수 있도록 정의하는 인터페이스입니다.
 */
UINTERFACE(BlueprintType)
class EMPTYCITY_API UIndicatorWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class IIndicatorWidgetInterface
{
	GENERATED_BODY()

public:
	/** 위젯에 IndicatorDescriptor를 연결하고 표시 정보를 갱신합니다. */
	UFUNCTION(BlueprintNativeEvent, Category = "Indicator")
	void BindIndicator(UIndicatorDescriptor* Indicator);

	/** 위젯에서 IndicatorDescriptor와 관련된 연결을 해제합니다. */
	UFUNCTION(BlueprintNativeEvent, Category = "Indicator")
	void UnbindIndicator(const UIndicatorDescriptor* Indicator);
};
