#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "ECQuickBarComponent.generated.h"

class UECInventoryItemInstance;
class UECEquipmentInstance;
class UECEquipmentManagerComponent;

/**
 * HUD QuickBar와 Inventory/Equipment 시스템을 연결하는 ControllerComponent입니다.
 *
 * [역할]
 * - PlayerController가 보유하며, "현재 선택된 슬롯"의 아이템을 Pawn에 장착/해제합니다.
 * - Slots 배열이 HUD에 표시되는 QuickBar 슬롯에 1:1로 대응합니다.
 * - 슬롯 변경 시 이전 장비를 해제하고 새 장비를 EquipmentManagerComponent에 요청합니다.
 *
 * UControllerComponent를 상속하므로 Pawn이 빙의(Possess)되기 전에도 동작할 수 있습니다.
 */
UCLASS()
class EMPTYCITY_API UECQuickBarComponent : public UControllerComponent
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// ControllerComponent Interface
// ─────────────────────────────────────────────────────────────
public:
	virtual void BeginPlay() override;
	

// ─────────────────────────────────────────────────────────────
// Slot
// ─────────────────────────────────────────────────────────────
public:
	/** 지정한 슬롯에 InventoryItemInstance를 등록합니다. (빈 슬롯에만 적용되며 덮어쓰기는 하지 않습니다) */
	void AddItemToSlot(int32 SlotIndex, UECInventoryItemInstance* Item);

	/** 활성 슬롯을 변경합니다. 기존 슬롯의 장비를 해제한 뒤 새 슬롯의 장비를 장착합니다. */
	void SetActiveSlotIndex(int32 NewIndex);

	/** 지정한 슬롯을 비우고 등록되어 있던 InventoryItemInstance를 반환합니다. (활성 슬롯이었다면 장비를 먼저 해제합니다) */
	UECInventoryItemInstance* RemoveItemFromSlot(int32 SlotIndex);

	/**
	 * 지정한 슬롯의 아이템을 "사용"합니다. (번호 키 입력 등의 진입점에서 호출)
	 * 슬롯의 ItemInstance를 payload로 담아 GameplayEvent.Item.Use 이벤트를 플레이어 ASC에 보내고,
	 * 실제 사용 로직(소모·효과 적용)은 이 이벤트로 발동되는 GA_UseItem이 담당합니다.
	 */
	void UseSlot(int32 SlotIndex);

	/** 전체 슬롯 배열의 복사본을 반환합니다. (HUD 갱신 등에서 슬롯 구성을 조회할 때 사용) */
	TArray<UECInventoryItemInstance*> GetSlots() const { return Slots; }

	/** 현재 활성화된 슬롯 인덱스를 반환합니다. (-1이면 선택 없음) */
	int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }

	/** 현재 활성 슬롯에 등록된 InventoryItemInstance를 반환합니다. (활성 슬롯이 없거나 비어 있으면 nullptr) */
	UECInventoryItemInstance* GetActiveSlotItem() const;

	
// ─────────────────────────────────────────────────────────────
// Equip / Unequip
// ─────────────────────────────────────────────────────────────
private:
	/** 현재 장착된 EquippedItem을 EquipmentManager를 통해 해제하고 nullptr로 초기화합니다. */
	void UnequipItemInSlot();

	/** 활성 슬롯의 아이템에서 EquippableItem Fragment를 찾아 EquipmentManager에 장착을 요청합니다. */
	void EquipItemInSlot();

	
// ─────────────────────────────────────────────────────────────
// Helper
// ─────────────────────────────────────────────────────────────
private:
	/** Controller가 빙의한 Pawn에서 EquipmentManagerComponent를 탐색해 반환합니다. */
	UECEquipmentManagerComponent* FindEquipmentManager() const;

	
// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
protected:
	/** HUD에 표시될 QuickBar 슬롯의 총 개수입니다. */
	UPROPERTY()
	int32 NumSlots = 2;

private:
	/** 각 슬롯에 등록된 InventoryItemInstance 목록입니다. (nullptr이면 빈 슬롯) */
	UPROPERTY()
	TArray<TObjectPtr<UECInventoryItemInstance>> Slots;

	/** 현재 활성화된 슬롯 인덱스입니다. (-1이면 선택 없음) */
	UPROPERTY()
	int32 ActiveSlotIndex = -1;

	/** 현재 슬롯에 의해 Pawn에 장착된 EquipmentInstance입니다. */
	UPROPERTY()
	TObjectPtr<UECEquipmentInstance> EquippedItem;

};
