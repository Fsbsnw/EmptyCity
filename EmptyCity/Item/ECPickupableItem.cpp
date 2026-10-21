#include "ECPickupableItem.h"
#include "Components/StaticMeshComponent.h"
#include "Data/Item/ECItemTableRow.h"
#include "ECGameplayAbility_PickupItem.h"
#include "Engine/StaticMesh.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "Net/UnrealNetwork.h"
#include "Subsystem/ECItemDataSubsystem.h"
#include "UObject/ConstructorHelpers.h"

AECPickupableItem::AECPickupableItem()
{
	bReplicates = true;
	SetReplicateMovement(false);

	// Item의 Mesh를 초기화 합니다.
	{
		Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
		Mesh->SetCollisionProfileName(TEXT("Interactable_OverlapDynamic"));
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

void AECPickupableItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StaticInventory);
}

void AECPickupableItem::SetPickupInventory(const FInventoryPickup& InPickupInventory)
{
	StaticInventory = InPickupInventory;
	ApplyWorldVisual();
}

void AECPickupableItem::OnRep_StaticInventory()
{
	ApplyWorldVisual();
}

void AECPickupableItem::GatherInteractionOptions(const FInteractionQuery& Query, FInteractionOptionBuilder& Builder)
{
	// 드롭된 아이템의 인디케이터는 인벤토리 Icon과 분리된 전용 필드만 사용합니다.
	if (!StaticInventory.Templates.IsEmpty() && StaticInventory.Templates[0].ItemDef)
	{
		if (const FECItemTableRow* ItemData =
			UECItemDataSubsystem::Get(this).FindItemData(StaticInventory.Templates[0].ItemDef))
		{
			Option.InteractionIcon = ItemData->InteractionIndicatorIcon;
		}
	}

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
	if (ItemData && !ItemData->WorldMesh.IsNull())
	{
		Mesh->SetStaticMesh(ItemData->WorldMesh.LoadSynchronous());
		Mesh->SetRelativeScale3D(ItemData->WorldScale);
	}
}
