// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ECRegionSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnRegionChanged,
	FGameplayTag,
	FGameplayTag
);

/**
 * 플레이어가 위치한 맵 지역 정보를 담당하는 서브시스템입니다.
 */
UCLASS()
class EMPTYCITY_API UECRegionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	static UECRegionSubsystem& Get(const UObject* WorldContextObject);
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 현재 지역을 새로운 지역으로 변경합니다. */
	UFUNCTION(BlueprintCallable)
	void EnterRegion(FGameplayTag NewRegionTag);

	/** 현재 상태에서 해당 지역에 진입할 수 있는지 반환합니다. */
	UFUNCTION(BlueprintCallable)
	bool CanEnterRegion(FGameplayTag NewRegionTag) const;

	/** 현재 지역 태그를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Region")
	FGameplayTag GetCurrentRegionTag() const { return CurrentRegionTag; }

	/** 이전 지역 태그를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Region")
	FGameplayTag GetPreviousRegionTag() const { return PreviousRegionTag; }

public:
	/** 지역이 변경될 때 호출됩니다. */
	FOnRegionChanged OnRegionChanged;

private:
	/** 현재 지역 태그입니다. */
	FGameplayTag CurrentRegionTag;

	/** 이전 지역 태그입니다. */
	FGameplayTag PreviousRegionTag;
};