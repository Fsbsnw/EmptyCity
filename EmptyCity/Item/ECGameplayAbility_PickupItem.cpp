#include "ECGameplayAbility_PickupItem.h"
#include "ECPickupableItem.h"
#include "ECLogChannels.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "Inventory/ECInventoryManagerComponent.h"

UECGameplayAbility_PickupItem::UECGameplayAbility_PickupItem()
{
	// 상호작용 키 입력을 통해 GameplayEvent로 발동되는 어빌리티입니다.
	ActivationPolicy = EAbilityActivationPolicy::OnInputTriggered;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UECGameplayAbility_PickupItem::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	LOG_ABILITY(TEXT("줍기 어빌리티 발동 - 인벤토리 보관을 시도합니다."));
	
	// 상호작용 어빌리티가 Payload.Target에 담아 보낸 "주울 아이템 액터"를 꺼냅니다.
	AActor* TargetActor = const_cast<AActor*>(ToRawPtr(TriggerEventData->Target));                                                                                                                                                       
	Pickupable = UPickupableStatics::GetFirstPickupableFromActor(TargetActor);
	
	PickUp();
	
	TargetActor->Destroy();

	// 즉시 처리되는 어빌리티이므로 곧바로 종료합니다.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UECGameplayAbility_PickupItem::PickUp()
{
	if (AController* Controller = GetECPlayerControllerFromActorInfo())
	{
		if (UECInventoryManagerComponent* InventoryComponent = Controller->GetComponentByClass<UECInventoryManagerComponent>())
		{
			UPickupableStatics::AddPickupToInventory(InventoryComponent, Pickupable);
		}
	}
}
