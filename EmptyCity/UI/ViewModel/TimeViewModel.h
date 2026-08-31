// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "TimeViewModel.generated.h"

enum class EECTimeOfDay : uint8;
/**
 * @brief 게임 시간 정보를 UI에 바인딩하기 위한 MVVM 뷰모델입니다.
 */
UCLASS()
class EMPTYCITY_API UTimeViewModel : public UECViewModelBase
{
	GENERATED_BODY()
public:
	virtual void BindCallbacksToDependencies(AActor* ContextActor) override;
	virtual void BroadcastInitialValues() override;

public: 
// ============================================================================
// MVVM 필드 노티파이용 Getters & Setters
// ============================================================================
	
	int32 GetCurrentDay() const { return CurrentDay; }
	void SetCurrentDay(int32 NewCurrentDay);
	
	FText GetCurrentTimeOfDay() const { return CurrentTimeOfDay; };
	void SetCurrentTimeOfDay(const FText& NewCurrentTimeOfDay);
	
private:
// ============================================================================
// View 바인딩 속성
// ============================================================================

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess))
	int32 CurrentDay;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess))
	FText CurrentTimeOfDay;

// ============================================================================
// View 바인딩 함수
// ============================================================================
	
	void HandleTimeChanged(int32 NewDay, EECTimeOfDay NewCurrentTimeOfDay);
};
