#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/ECGameplayAbility.h"
#include "ECGameplayAbility_PickupItem.generated.h"

class IPickupable;
/**
 * 떨어진 아이템(AECPickupableItem)을 주워 인벤토리에 보관하는 어빌리티입니다.
 *
 * 이 어빌리티는 입력으로 직접 발동하지 않고, 상호작용 시스템이 대신 발동시킵니다.
 *  1. 플레이어가 아이템에 다가가면 GrantNearbyInteraction 태스크가 이 어빌리티를 플레이어 ASC에 부여합니다.
 *  2. 플레이어가 상호작용 키를 누르면 UECGameplayAbility_Interact가 GameplayEvent로 이 어빌리티를 발동합니다.
 *     이때 EventData.Target에 주울 아이템 액터가 담겨 옵니다.
 *  3. 아이템의 설계도(ItemDef)를 인벤토리에 추가하고, 월드에 있던 아이템 액터를 제거합니다.
 */
UCLASS()
class EMPTYCITY_API UECGameplayAbility_PickupItem : public UECGameplayAbility
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// GameplayAbility Interface
// ─────────────────────────────────────────────────────────────
public:
	UECGameplayAbility_PickupItem();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	
// ─────────────────────────────────────────────────────────────
// Pickup
// ─────────────────────────────────────────────────────────────
protected:
	/** 캐싱해 둔 Pickupable 대상의 내용물을 플레이어 인벤토리에 추가합니다. */
	void PickUp();

	
// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
private:
	/** 발동 시점에 대상 액터로부터 찾아 둔 Pickupable 대상입니다. (PickUp()에서 인벤토리에 추가) */
	UPROPERTY()
	TScriptInterface<IPickupable> Pickupable;

};
