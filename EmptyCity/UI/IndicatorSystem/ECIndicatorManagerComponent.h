// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ControllerComponent.h"

#include "ECIndicatorManagerComponent.generated.h"

class AController;
class UIndicatorDescriptor;
class UObject;
struct FFrame;

/**
 * 컨트롤러가 관리하는 인디케이터 목록을 보관하고 추가·제거 이벤트를 전달합니다
 */
UCLASS(BlueprintType, Blueprintable)
class EMPTYCITY_API UECIndicatorManagerComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	/** IndicatorManagerComponent를 생성하고 기본값을 초기화합니다. */
	UECIndicatorManagerComponent(const FObjectInitializer& ObjectInitializer);

	/** 지정한 컨트롤러에서 IndicatorManagerComponent를 찾아 반환합니다. */
	static UECIndicatorManagerComponent* GetComponent(AController* Controller);

	/** Indicator를 관리 목록에 추가하고 OnIndicatorAdded 이벤트를 호출합니다. */
	UFUNCTION(BlueprintCallable, Category = Indicator)
	void AddIndicator(UIndicatorDescriptor* IndicatorDescriptor);

	/** Indicator를 관리 목록에서 제거하고 OnIndicatorRemoved 이벤트를 호출합니다. */
	UFUNCTION(BlueprintCallable, Category = Indicator)
	void RemoveIndicator(UIndicatorDescriptor* IndicatorDescriptor);

	/** Indicator가 추가되거나 제거될 때 Descriptor를 전달하는 이벤트 형식을 정의합니다. */
	DECLARE_EVENT_OneParam(
		UECIndicatorManagerComponent,
		FIndicatorEvent,
		UIndicatorDescriptor* Descriptor
	)

	/** Indicator가 관리 목록에 추가되었을 때 호출합니다. */
	FIndicatorEvent OnIndicatorAdded;

	/** Indicator가 관리 목록에서 제거되었을 때 호출합니다. */
	FIndicatorEvent OnIndicatorRemoved;

	/** 현재 관리 중인 전체 Indicator 목록을 반환합니다. */
	const TArray<UIndicatorDescriptor*>& GetIndicators() const
	{
		return Indicators;
	}

private:
	/** 현재 등록되어 있으며 수명을 유지해야 하는 Indicator Descriptor 목록을 보관합니다. */
	UPROPERTY()
	TArray<TObjectPtr<UIndicatorDescriptor>> Indicators;
};
