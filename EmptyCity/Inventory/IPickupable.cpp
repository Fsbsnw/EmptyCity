#include "IPickupable.h"
#include "Data/Item/ECItemTableRow.h"
#include "GameFramework/Actor.h"
#include "ECInventoryItemDefinition.h"
#include "ECInventoryManagerComponent.h"
#include "ECInventoryItemInstance.h"
#include "Subsystem/ECItemDataSubsystem.h"
#include "UObject/ScriptInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IPickupable)

class UActorComponent;

namespace
{
struct FECSimulatedInventoryEntry
{
	TSubclassOf<UECInventoryItemDefinition> ItemDef;
	int32 StackCount = 0;
};

bool CanAddPickupToInventory(const UECInventoryManagerComponent* InventoryComponent, const FInventoryPickup& PickupInventory)
{
	TArray<FECSimulatedInventoryEntry> SimulatedEntries;
	SimulatedEntries.Reserve(UECInventoryManagerComponent::MaxInventorySlots);
	for (const FInventoryEntry& Entry : InventoryComponent->InventoryList.Entries)
	{
		FECSimulatedInventoryEntry& SimulatedEntry = SimulatedEntries.AddDefaulted_GetRef();
		SimulatedEntry.ItemDef = Entry.Instance->ItemDef;
		SimulatedEntry.StackCount = Entry.StackCount;
	}

	for (const FPickupTemplate& Template : PickupInventory.Templates)
	{
		const FECItemTableRow* ItemData = UECItemDataSubsystem::Get(InventoryComponent).FindItemData(Template.ItemDef);
		const int32 StackMax = FMath::Max(1, ItemData->StackMax);
		int32 Remaining = Template.StackCount;
		for (FECSimulatedInventoryEntry& Entry : SimulatedEntries)
		{
			if (Entry.ItemDef != Template.ItemDef || Entry.StackCount >= StackMax) continue;
			const int32 AddedCount = FMath::Min(StackMax - Entry.StackCount, Remaining);
			Entry.StackCount += AddedCount;
			Remaining -= AddedCount;
			if (Remaining == 0) break;
		}

		while (Remaining > 0 && SimulatedEntries.Num() < UECInventoryManagerComponent::MaxInventorySlots)
		{
			FECSimulatedInventoryEntry& NewEntry = SimulatedEntries.AddDefaulted_GetRef();
			NewEntry.ItemDef = Template.ItemDef;
			NewEntry.StackCount = FMath::Min(StackMax, Remaining);
			Remaining -= NewEntry.StackCount;
		}

		if (Remaining > 0) return false;
	}

	return true;
}
}

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
	TArray<UActorComponent*> PickupableComponents = Actor->GetComponentsByInterface(UPickupable::StaticClass());
	if (PickupableComponents.Num() > 0)
	{
		// 첫 번째 대상만 반환합니다. 더 정교한 선택이 필요하면 호출하는 쪽에서 별도로 처리해야 합니다.
		return TScriptInterface<IPickupable>(PickupableComponents[0]);
	}

	return TScriptInterface<IPickupable>();
}

bool UPickupableStatics::AddPickupToInventory(UECInventoryManagerComponent* InventoryComponent, TScriptInterface<IPickupable> Pickup)
{
	const FInventoryPickup& PickupInventory = Pickup->GetPickupInventory();
	if (!CanAddPickupToInventory(InventoryComponent, PickupInventory)) return false;

	// 설계도 묶음: 설계도와 개수로 인벤토리에서 새 인스턴스를 생성합니다.
	for (const FPickupTemplate& Template : PickupInventory.Templates)
	{
		InventoryComponent->AddItemDefinition(Template.ItemDef, Template.StackCount);
	}

	return true;
}
