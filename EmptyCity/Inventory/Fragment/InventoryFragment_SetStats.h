#pragma once

#include "GameplayTagContainer.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "InventoryFragment_SetStats.generated.h"

class UECInventoryItemInstance;
class UObject;
struct FGameplayTag;

/**
 * 아이템이 처음 생성될 때 초기 스탯(태그→개수)을 인스턴스에 채워 주는 Fragment입니다.
 *
 * [동작 흐름]
 * 1. 에디터에서 InitialItemStats에 "어떤 태그를 몇 개로 시작할지" 미리 채워 둡니다. (예: 탄약 30, 수량 1)
 * 2. 인벤토리에 아이템이 추가되면(InventoryManagerComponent의 AddEntry) OnInstanceCreated가 호출됩니다.
 * 3. 이때 InitialItemStats의 값들을 인스턴스의 StatTags에 그대로 쌓아 초기 상태를 만듭니다.
 *
 * 즉 이 Fragment(설계도)는 "시작 수치"만 정의하고, 실제 런타임 수치는 인스턴스의 StatTags가 보관합니다.
 */
UCLASS()
class UInventoryFragment_SetStats : public UECInventoryItemFragment
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Fragment Interface
// ─────────────────────────────────────────────────────────────
public:
	/** 아이템 인스턴스가 막 생성될 때 호출되어, InitialItemStats의 값들을 인스턴스의 StatTags에 채워 넣습니다. */
	virtual void OnInstanceCreated(UECInventoryItemInstance* Instance) const override;

	
// ─────────────────────────────────────────────────────────────
// Query
// ─────────────────────────────────────────────────────────────
public:
	/** 이 설계도에 정의된 지정 태그의 초기 스탯 값을 반환합니다. (정의되지 않은 태그면 0) */
	int32 GetItemStatByTag(FGameplayTag Tag) const;

	
// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
protected:
	/** 아이템 생성 시 부여할 초기 스탯 목록입니다. (태그 → 시작 개수) 에디터에서 미리 채워 둡니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수")
	TMap<FGameplayTag, int32> InitialItemStats;

};
