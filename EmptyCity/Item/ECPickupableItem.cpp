#include "ECPickupableItem.h"
#include "Components/StaticMeshComponent.h"
#include "Data/Item/ECItemTableRow.h"
#include "ECGameplayAbility_PickupItem.h"
#include "Engine/StaticMesh.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "Subsystem/ECItemDataSubsystem.h"
#include "UObject/ConstructorHelpers.h"

AECPickupableItem::AECPickupableItem()
{
	// Item의 Mesh를 초기화 합니다.
	{
		Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
		Mesh->SetCollisionProfileName(TEXT("Interactable_BlockDynamic"));	
		SetRootComponent(Mesh);
		static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultPickupMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
		Mesh->SetStaticMesh(DefaultPickupMesh.Object);
		Mesh->SetRelativeScale3D(FVector(0.25f));
	}

	Option.Text = NSLOCTEXT("EmptyCity", "PickupItem", "줍기");
	Option.InteractionAbilityToGrant = UECGameplayAbility_PickupItem::StaticClass();
}

void AECPickupableItem::BeginPlay()
{
	Super::BeginPlay();
	ApplyWorldVisual();
}

void AECPickupableItem::SetPickupInventory(const FInventoryPickup& InPickupInventory)
{
	StaticInventory = InPickupInventory;
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

void AECPickupableItem::ApplyWorldVisual()
{
	if (!Mesh || StaticInventory.Templates.IsEmpty() || !StaticInventory.Templates[0].ItemDef)
	{
		return;
	}

	const FECItemTableRow* ItemData = UECItemDataSubsystem::Get(this).FindItemData(StaticInventory.Templates[0].ItemDef);
	if (!ItemData->WorldMesh.IsNull())
	{
		Mesh->SetStaticMesh(ItemData->WorldMesh.LoadSynchronous());
		Mesh->SetRelativeScale3D(ItemData->WorldScale);
	}
}
