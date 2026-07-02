#pragma once

#include "CoreMinimal.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "InventoryFragment_EquippableItem.generated.h"

class UECEquipmentDefinition;

/**
 * "이 아이템은 장착할 수 있다"는 사실과, 장착 시 사용할 설계도를 담는 Fragment입니다.
 *
 * 주의: 이 Fragment 자체는 장착을 수행하지 않습니다. "장착 가능하다는 표식 + 어떤 장비로 장착할지"만 들고 있습니다.
 * 실제 흐름은 QuickBarComponent가 슬롯 활성화 시 이 Fragment의 존재를 확인하고,
 * 여기 담긴 EquipmentDefinition을 꺼내 EquipmentManagerComponent에 장착을 요청하는 식으로 이어집니다.
 */
UCLASS()
class EMPTYCITY_API UInventoryFragment_EquippableItem : public UECInventoryItemFragment
{
	GENERATED_BODY()

public:
	/** 이 아이템을 장착할 때 사용할 장비 설계도입니다. (어떤 Actor/Ability로 장착되는지가 여기에 정의됨) */
	UPROPERTY(EditDefaultsOnly, Category = "변수")
	TSubclassOf<UECEquipmentDefinition> EquipmentDefinition;

};
