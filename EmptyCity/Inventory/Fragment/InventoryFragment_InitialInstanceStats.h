#pragma once

#include "GameplayTagContainer.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "InventoryFragment_InitialInstanceStats.generated.h"

class UECInventoryItemInstance;

/**
 * 아이템 인스턴스가 생성될 때 인스턴스별 초기 상태를 StatTags에 설정하는 Fragment입니다.
 *
 * 무기 내구도, 탄약, 충전 횟수, 품질 및 강화 단계처럼 같은 ItemDefinition을 사용하는
 * 인스턴스마다 달라질 수 있는 초기값을 정의합니다. 인벤토리 스택 수량은 이 Fragment가 아닌
 * FInventoryEntry::StackCount에서 관리합니다.
 */
UCLASS(DisplayName = "인스턴스 초기 스탯")
class EMPTYCITY_API UInventoryFragment_InitialInstanceStats : public UECInventoryItemFragment
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Fragment Interface
// ─────────────────────────────────────────────────────────────
public:
	/** 새 아이템 인스턴스의 StatTags에 InitialStats를 적용합니다. */
	virtual void OnInstanceCreated(UECInventoryItemInstance* Instance) const override;

// ─────────────────────────────────────────────────────────────
// Query
// ─────────────────────────────────────────────────────────────
public:
	/** 지정한 태그의 초기 스탯 값을 반환합니다. 정의되지 않은 태그면 0을 반환합니다. */
	int32 GetItemStatByTag(FGameplayTag Tag) const;

// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
protected:
	/** 인스턴스 생성 시 부여할 초기 상태입니다. 인벤토리 StackCount는 이 맵에 저장하지 않습니다. */
	UPROPERTY(EditDefaultsOnly, Category = "인스턴스", meta = (DisplayName = "초기 스탯"))
	TMap<FGameplayTag, int32> InitialStats;
};
