// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "BedMenuViewModel.generated.h"

class AECBed;
enum class EECTimeOfDay : uint8;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UBedMenuViewModel : public UECViewModelBase
{
	GENERATED_BODY()
public:
	virtual void BindCallbacksToDependencies(AActor* ContextActor) override;
	virtual void BroadcastInitialValues() override;

	int32 GetCurrentDay() const { return CurrentDay; }
	void SetCurrentDay(int32 NewCurrentDay);

	int32 GetWakeUpDay() const { return WakeUpDay; }
	void SetWakeUpDay(int32 NewWakeUpDay);

	/** 수면을 실행합니다. */
	UFUNCTION(BlueprintCallable)
	void RequestSleepSequence();

	/** 시간 변경 없이 침대 메뉴만 닫습니다. */
	UFUNCTION(BlueprintCallable)
	void RequestCloseMenu();

private:
	/** 시간 변경 시 현재 날짜와 기상 예정 날짜를 갱신합니다. */
	void HandleTimeChanged(int32 NewDay, EECTimeOfDay NewTimeOfDay);

	/** 침대 메뉴를 연 현재 날짜입니다. */
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"))
	int32 CurrentDay = 1;

	/** 수면 후 기상하게 될 날짜입니다. */
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"))
	int32 WakeUpDay = 2;

	TObjectPtr<AECBed> TargetBed = nullptr;
};
