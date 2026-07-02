#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/ECGameplayAbility.h"
#include "ECGameplayAbility_FromEquipment.generated.h"

class UECEquipmentInstance;

/**
 * 장비(Equipment)를 통해 부여되는 어빌리티들의 베이스 클래스입니다.
 *
 * AbilitySet으로 어빌리티를 부여할 때 EquipmentInstance를 SourceObject로 넘겨 두는데,
 * 이 클래스는 그 SourceObject를 다시 꺼내 "이 어빌리티가 어떤 장비에서 나온 것인지"를 알 수 있게 해 줍니다.
 * (예: 발사 어빌리티에서 자신을 발사한 무기 인스턴스에 접근)
 */
UCLASS()
class EMPTYCITY_API UECGameplayAbility_FromEquipment : public UECGameplayAbility
{
	GENERATED_BODY()

public:
	/** 장착 중에만 존재하는 런타임 Equipment 객체를 반환합니다. */
	UECEquipmentInstance* GetAssociatedEquipment() const;
	
	/** 인벤토리에 들어있는 아이템 원본 객체를 반환합니다. StatTags를 접근할 수 있습니다. */
	UECEquipmentInstance* GetAssociatedItem() const;

};
