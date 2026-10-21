#include "IPickupable.h"
#include "GameFramework/Actor.h"
#include "ECInventoryItemDefinition.h"
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
	TArray<UActorComponent*> PickupableComponents = Actor->GetComponentsByInterface(UPickupable::StaticClass());
	if (PickupableComponents.Num() > 0)
	{
		// 첫 번째 대상만 반환합니다. 더 정교한 선택이 필요하면 호출하는 쪽에서 별도로 처리해야 합니다.
		return TScriptInterface<IPickupable>(PickupableComponents[0]);
	}

	return TScriptInterface<IPickupable>();
}

EECPickupResult UPickupableStatics::AddPickupToInventory(
	UECInventoryManagerComponent* InventoryComponent,
	TScriptInterface<IPickupable> Pickup)
{
	if (!InventoryComponent || !Pickup)
	{
		return EECPickupResult::Failed;
	}

	const FInventoryPickup& PickupInventory = Pickup->GetPickupInventory();
	if (PickupInventory.Templates.IsEmpty())
	{
		return EECPickupResult::Failed;
	}

	// 잘못된 데이터 때문에 일부만 추가되는 상황을 막기 위해 변경 전에 전체를 검증합니다.
	for (const FPickupTemplate& Template : PickupInventory.Templates)
	{
		if (!Template.ItemDef || Template.StackCount <= 0)
		{
			return EECPickupResult::Failed;
		}
	}

	FInventoryPickup RemainingPickup;
	int64 TotalAddedCount = 0;
	bool bBlockedByWeight = false;

	for (const FPickupTemplate& Template : PickupInventory.Templates)
	{
		const int32 AddableCount = InventoryComponent->GetMaxAddableItemCount(
			Template.ItemDef,
			Template.StackCount);
		if (AddableCount <= 0)
		{
			const double UnitWeight = InventoryComponent->GetItemDefinitionUnitWeight(Template.ItemDef);
			const double MaxWeight = InventoryComponent->GetMaxInventoryWeight();
			bBlockedByWeight |= MaxWeight > 0.0
				&& UnitWeight > UE_SMALL_NUMBER
				&& static_cast<double>(InventoryComponent->GetTotalWeight()) + UnitWeight
					> MaxWeight + UE_KINDA_SMALL_NUMBER;
		}

		int32 AddedCount = 0;
		if (AddableCount > 0
			&& InventoryComponent->AddItemDefinition(Template.ItemDef, AddableCount))
		{
			AddedCount = AddableCount;
			TotalAddedCount += AddedCount;
		}

		const int32 RemainingCount = Template.StackCount - AddedCount;
		if (RemainingCount > 0)
		{
			FPickupTemplate& RemainingTemplate = RemainingPickup.Templates.AddDefaulted_GetRef();
			RemainingTemplate.ItemDef = Template.ItemDef;
			RemainingTemplate.StackCount = RemainingCount;
		}
	}

	if (TotalAddedCount <= 0)
	{
		return bBlockedByWeight
			? EECPickupResult::FailedWeightLimit
			: EECPickupResult::Failed;
	}

	Pickup->SetPickupInventory(RemainingPickup);
	return RemainingPickup.Templates.IsEmpty()
		? EECPickupResult::Complete
		: EECPickupResult::Partial;
}
