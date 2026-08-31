#include "ECItemDropComponent.h"
#include "Data/Item/ECItemDropTableRow.h"
#include "Data/Item/ECItemTableRow.h"
#include "ECPickupableItem.h"
#include "Engine/World.h"
#include "Inventory/ECInventoryItemDefinition.h"

namespace
{
constexpr float DropSpawnRadius = 75.0f;
constexpr float DropSpawnHeight = 25.0f;
}

UECItemDropComponent::UECItemDropComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FInventoryPickup UECItemDropComponent::GenerateDrop() const
{
	const FECItemDropTableRow& DropData = *DropTableRow.GetRow<FECItemDropTableRow>(TEXT("UECItemDropComponent::GenerateDrop"));
	FInventoryPickup Pickup;

	// 각 드롭 항목의 확률과 수량을 독립적으로 계산합니다.
	for (const FECItemDropEntry& DropEntry : DropData.DropEntries)
	{
		if (FMath::RandRange(1, 100) > DropEntry.DropChance) continue;
		const FECItemTableRow& ItemData = *DropEntry.Item.GetRow<FECItemTableRow>(TEXT("UECItemDropComponent::GenerateDrop"));
		FPickupTemplate& PickupTemplate = Pickup.Templates.AddDefaulted_GetRef();
		PickupTemplate.ItemDef = ItemData.ItemDefinition.LoadSynchronous();
		PickupTemplate.StackCount = FMath::RandRange(DropEntry.MinCount, DropEntry.MaxCount);
	}
	return Pickup;
}

void UECItemDropComponent::SpawnDrop() const
{
	UWorld* World = GetWorld();
	if (World->bIsTearingDown) return;

	// 컴포넌트에 설정된 드롭 행으로 생성할 아이템 목록을 계산합니다.
	const FInventoryPickup DropResult = GenerateDrop();
	AActor* OwnerActor = GetOwner();
	FVector BoundsOrigin;
	FVector BoundsExtent;
	OwnerActor->GetActorBounds(false, BoundsOrigin, BoundsExtent);
	const FVector DropOrigin(OwnerActor->GetActorLocation().X, OwnerActor->GetActorLocation().Y, BoundsOrigin.Z - BoundsExtent.Z);

	// 계산된 아이템을 소유 액터 주변에 원형으로 배치합니다.
	for (int32 DropIndex = 0; DropIndex < DropResult.Templates.Num(); ++DropIndex)
	{
		const float DropAngle = UE_TWO_PI * DropIndex / DropResult.Templates.Num();
		const FVector DropOffset(FMath::Cos(DropAngle) * DropSpawnRadius, FMath::Sin(DropAngle) * DropSpawnRadius, DropSpawnHeight);
		const FTransform DropTransform(OwnerActor->GetActorRotation(), DropOrigin + DropOffset);
		AECPickupableItem* PickupActor = World->SpawnActorDeferred<AECPickupableItem>(AECPickupableItem::StaticClass(), DropTransform, OwnerActor, OwnerActor->GetInstigator(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		FInventoryPickup PickupInventory;
		PickupInventory.Templates.Add(DropResult.Templates[DropIndex]);
		PickupActor->SetPickupInventory(PickupInventory);
		PickupActor->FinishSpawning(DropTransform);
	}
}
