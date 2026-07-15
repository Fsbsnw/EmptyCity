// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/InventoryContainer/ECInventoryContainer.h"

#include "Character/Player/ECPlayer.h"
#include "Components/BoxComponent.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "UI/Subsystem/UIManagerSubsystem.h"

void AECInventoryContainer::GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& OptionBuilder)
{
	OptionBuilder.AddInteractionOption(Option);
}

UECInventoryManagerComponent* AECInventoryContainer::GetInventoryManagerComponent() const
{
	return InventoryManagerComponent;
}

AECInventoryContainer::AECInventoryContainer()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	SetRootComponent(Mesh);
    
	BoxCollision = CreateDefaultSubobject<UBoxComponent>("Box Collision");
	BoxCollision->SetupAttachment(Mesh);
	
	// InventoryManager를 초기화합니다.
	{
		InventoryManagerComponent = CreateDefaultSubobject<UECInventoryManagerComponent>(TEXT("InventoryManagerComponent"));
	}
}

void AECInventoryContainer::BeginPlay()
{
	Super::BeginPlay();

	AddInitialInventory();

	BoxCollision->OnComponentEndOverlap.AddDynamic(this, &ThisClass::HideInventory);
}

UUIManagerSubsystem* AECInventoryContainer::GetActiveUIManager() const
{
	return UUIManagerSubsystem::Get(ActivePlayerController.Get());
}

void AECInventoryContainer::ShowInventory(APlayerController* PC)
{
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(PC))
	{
		ActivePlayerController = PC;
        
		UIManager->OpenDynamicWidget(InventoryUITag, InventoryWidget, this);
	}
}

void AECInventoryContainer::HideInventory(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	if (PlayerPawn == nullptr)
	{
		return;
	}

	APlayerController* LeavingPC = Cast<APlayerController>(PlayerPawn->GetController());

	if (LeavingPC == nullptr || LeavingPC != ActivePlayerController.Get())
	{
		return;
	}

	if (UUIManagerSubsystem* UIManager = GetActiveUIManager())
	{
		UIManager->CloseDynamicWidget(InventoryUITag);
	}

	ActivePlayerController.Reset();
}

void AECInventoryContainer::AddInitialInventory()
{
	if (InitialInventoryItems.IsEmpty()) return;
	if (!InventoryManagerComponent) return;

	for (const FInitialInventoryItem& InitialItem : InitialInventoryItems)
	{
		if (!InitialItem.ItemDef) continue;

		// 인벤토리에 아이템을 지급한다.
		// @TODO: Inventory UI 구현시 ItemInstance를 UI에 추가할 것
		UECInventoryItemInstance* ItemInstance = InventoryManagerComponent->AddItemDefinition(InitialItem.ItemDef, InitialItem.StackCount);
	}
}