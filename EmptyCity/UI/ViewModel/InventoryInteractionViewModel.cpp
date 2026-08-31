#include "UI/ViewModel/InventoryInteractionViewModel.h"

#include "Actor/InventoryContainer/ECInventoryContainer.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "UI/ViewModel/InventoryViewModel.h"
#include "UI/Widget/ECUserWidget.h"

void UInventoryInteractionViewModel::BindCallbacksToDependencies(AActor* ContextActor)
{
    Super::BindCallbacksToDependencies(ContextActor);

    PlayerInventoryComponent.Reset();
    StorageInventoryComponent.Reset();

    PlayerInventoryViewModel = nullptr;
    StorageInventoryViewModel = nullptr;

    /*
     * 플레이어 인벤토리 검색
     */
    if (const UECUserWidget* OwningWidget = GetTypedOuter<UECUserWidget>())
    {
        if (AECPlayerController* PlayerController = Cast<AECPlayerController>(OwningWidget->GetOwningPlayer()))
        {
            PlayerInventoryComponent = PlayerController->GetInventoryManagerComponent();
        }
    }

    /*
     * 저장고 인벤토리 검색
     */
    if (AECInventoryContainer* Storage = Cast<AECInventoryContainer>(ContextActor))
    {
        StorageInventoryComponent = Storage->GetInventoryManagerComponent();
    }

    /*
     * 플레이어 기본 ViewModel 생성
     */
    if (UECInventoryManagerComponent* PlayerInventory = PlayerInventoryComponent.Get())
    {
        PlayerInventoryViewModel = NewObject<UInventoryViewModel>(this);
        PlayerInventoryViewModel->Initialize(PlayerInventory);
    }

    /*
     * 저장고 기본 ViewModel 생성
     */
    if (UECInventoryManagerComponent* StorageInventory = StorageInventoryComponent.Get())
    {
        StorageInventoryViewModel = NewObject<UInventoryViewModel>(this);
        StorageInventoryViewModel->Initialize(StorageInventory);
    }
}

void UInventoryInteractionViewModel::BroadcastInitialValues()
{
    Super::BroadcastInitialValues();

    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetPlayerInventoryViewModel);
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStorageInventoryViewModel);
    
    if (PlayerInventoryViewModel)
    {
        PlayerInventoryViewModel->BroadcastInitialValues();
    }

    if (StorageInventoryViewModel)
    {
        StorageInventoryViewModel->BroadcastInitialValues();
    }
}

bool UInventoryInteractionViewModel::GetCanTransferItems() const
{
    const UECInventoryManagerComponent* PlayerInventory = PlayerInventoryComponent.Get();
    const UECInventoryManagerComponent* StorageInventory = StorageInventoryComponent.Get();

    return PlayerInventory && StorageInventory && PlayerInventory != StorageInventory;
}

int32 UInventoryInteractionViewModel::MoveAllItemsToStorage()
{
    UECInventoryManagerComponent* PlayerInventory = PlayerInventoryComponent.Get();
    UECInventoryManagerComponent* StorageInventory = StorageInventoryComponent.Get();

    if (!PlayerInventory || !StorageInventory)
    {
        return 0;
    }

    return PlayerInventory->TransferAllItemsTo(StorageInventory);
}

int32 UInventoryInteractionViewModel::MoveItemToStorage(UECInventoryItemInstance* ItemInstance, int32 Count)
{
    UECInventoryManagerComponent* PlayerInventory = PlayerInventoryComponent.Get();
    UECInventoryManagerComponent* StorageInventory = StorageInventoryComponent.Get();

    if (!PlayerInventory || !StorageInventory || !IsValid(ItemInstance) || Count <= 0)
    {
        return 0;
    }

    return PlayerInventory->TransferItemTo(StorageInventory, ItemInstance, Count);
}

int32 UInventoryInteractionViewModel::MoveItemToPlayer(UECInventoryItemInstance* ItemInstance, int32 Count)
{
    UECInventoryManagerComponent* PlayerInventory = PlayerInventoryComponent.Get();
    UECInventoryManagerComponent* StorageInventory = StorageInventoryComponent.Get();

    if (!PlayerInventory || !StorageInventory || !IsValid(ItemInstance) || Count <= 0)
    {
        return 0;
    }

    return StorageInventory->TransferItemTo(PlayerInventory, ItemInstance, Count);
}

int32 UInventoryInteractionViewModel::DiscardPlayerItem(UECInventoryItemInstance* ItemInstance, int32 Count)
{
    UECInventoryManagerComponent* PlayerInventory = PlayerInventoryComponent.Get();

    if (!PlayerInventory || !IsValid(ItemInstance) || Count <= 0)
    {
        return 0;
    }

    return PlayerInventory->RemoveItemCount(ItemInstance, Count);
}

int32 UInventoryInteractionViewModel::DiscardStorageItem(UECInventoryItemInstance* ItemInstance, int32 Count)
{
    UECInventoryManagerComponent* StorageInventory = StorageInventoryComponent.Get();

    if (!StorageInventory || !IsValid(ItemInstance) || Count <= 0)
    {
        return 0;
    }

    return StorageInventory->RemoveItemCount(ItemInstance, Count);
}