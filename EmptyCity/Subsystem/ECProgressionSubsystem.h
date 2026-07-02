// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ECProgressionSubsystem.generated.h"

class UECInventoryItemDefinition;
/**
 * @brief 게임 내 진행도와 관련된 데이터를 관리하는 서브시스템입니다.
 * 맵의 특정 지역, 아이템 청사진의 해금과 관련된 정보를 저장하고 연출을
 * 실행할 대기열에 등록하여 관리합니다.
 */
UCLASS()
class EMPTYCITY_API UECProgressionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	/** 기본 해금 상태를 프로젝트 설정에서 가져옵니다. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
public:
	/** 진행도 해금 상태를 확인합니다. */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	bool IsContentUnlocked(FGameplayTag Tag) const;

	/** 아이템의 해금 상태를 프래그먼트에서 확인합니다. */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	bool IsItemUnlocked(TSubclassOf<UECInventoryItemDefinition> ItemClass) const;
	
	/** 진행도 해금 시에 호출(맵, 아이템 청사진 등)되는 함수입니다. */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void UnlockContent(FGameplayTag UnlockTag);

	/** 진행도 잠금 시에 호출되는 함수입니다. */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void LockContent(FGameplayTag LockTag);

	/** UI를 열었을 때, 카메라 연출을 해야 할 데이터가 있는지 확인하고 '읽음 처리'합니다. */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	bool CheckAndConsumeReveal(FGameplayTag Tag);

private:
	/** 해금된 정보를 관리합니다. */
	FGameplayTagContainer UnlockedTags;

	/** 아직 카메라 연출이 실행되지 않은 '새로운' 해금 대기열을 관리합니다. (연출을 보면 지워짐) */
	FGameplayTagContainer PendingRevealTags;
};
