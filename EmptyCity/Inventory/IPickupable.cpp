#include "IPickupable.h"
#include "GameFramework/Actor.h"
#include "ECInventoryManagerComponent.h"
#include "UObject/ScriptInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IPickupable)

class UActorComponent;

UPickupableStatics::UPickupableStatics()
	: Super(FObjectInitializer::Get())
{
}

TScriptInterface<IPickupable> UPickupableStatics::GetFirstPickupableFromActor(AActor* Actor)
{
	// 액터 자신이 곧바로 Pickupable이면 그것을 반환합니다.
	TScriptInterface<IPickupable> PickupableActor(Actor);
	if (PickupableActor)
	{
		return PickupableActor;
	}

	// 액터 자신이 아니라면, Pickupable을 구현한 컴포넌트가 붙어 있을 수 있으므로 그쪽을 찾습니다.
	TArray<UActorComponent*> PickupableComponents = Actor ? Actor->GetComponentsByInterface(UPickupable::StaticClass()) : TArray<UActorComponent*>();
	if (PickupableComponents.Num() > 0)
	{
		// 첫 번째 대상만 반환합니다. 더 정교한 선택이 필요하면 호출하는 쪽에서 별도로 처리해야 합니다.
		return TScriptInterface<IPickupable>(PickupableComponents[0]);
	}

	return TScriptInterface<IPickupable>();
}

void UPickupableStatics::AddPickupToInventory(UECInventoryManagerComponent* InventoryComponent, TScriptInterface<IPickupable> Pickup)
{
	if (InventoryComponent && Pickup)
	{
		const FInventoryPickup& PickupInventory = Pickup->GetPickupInventory();

		// 설계도 묶음: 설계도와 개수로 인벤토리에서 새 인스턴스를 생성합니다.
		for (const FPickupTemplate& Template : PickupInventory.Templates)
		{
			InventoryComponent->AddItemDefinition(Template.ItemDef, Template.StackCount);
		}
	}
}
