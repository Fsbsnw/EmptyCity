#include "ECPickupableItem.h"
#include "Components/StaticMeshComponent.h"

AECPickupableItem::AECPickupableItem()
{
	// Item의 Mesh를 초기화 합니다.
	{
		Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
		Mesh->SetCollisionProfileName(TEXT("Interactable_BlockDynamic"));	
		SetRootComponent(Mesh);
	}
}

void AECPickupableItem::GatherInteractionOptions(const FInteractionQuery& Query, FInteractionOptionBuilder& Builder)
{
	// 옵션을 추가하면 Builder가 InteractableTarget 필드를 이 액터로 자동 채워줍니다.
	Builder.AddInteractionOption(Option);
}

FInventoryPickup AECPickupableItem::GetPickupInventory() const
{
	return StaticInventory;
}
