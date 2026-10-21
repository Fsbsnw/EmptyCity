#include "UI/ViewModel/InventoryViewModel.h"

#include "Inventory/ECInventoryItemInstance.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "Subsystem/ECItemDataSubsystem.h"

namespace
{
    const TArray<FInventoryEntry>& GetEmptyInventoryEntries()
    {
        static const TArray<FInventoryEntry> EmptyEntries;
        return EmptyEntries;
    }
}

void UInventoryViewModel::Initialize(UECInventoryManagerComponent* InInventoryComponent)
{
    /*
     * 같은 ViewModel을 다른 인벤토리로 재설정할 경우를 대비해
     * 기존 델리게이트 연결을 제거합니다.
     */
    if (UECInventoryManagerComponent* PreviousInventory = InventoryComponent.Get())
    {
        PreviousInventory->OnInventoryUpdated.RemoveAll(this);
    }

    InventoryComponent = InInventoryComponent;

    if (UECInventoryManagerComponent* Inventory = InventoryComponent.Get())
    {
        Inventory->OnInventoryUpdated.AddUObject(this, &ThisClass::OnInventoryUpdated);
    }

    OnVMInventoryUpdated.Broadcast();
}

void UInventoryViewModel::BroadcastInitialValues()
{
    Super::BroadcastInitialValues();

    OnVMInventoryUpdated.Broadcast();
}

const TArray<FInventoryEntry>& UInventoryViewModel::GetInventoryEntries() const
{
    const UECInventoryManagerComponent* Inventory = InventoryComponent.Get();

    return Inventory ? Inventory->InventoryList.Entries : GetEmptyInventoryEntries();
}

TArray<FInventoryEntry> UInventoryViewModel::GetInventoryEntriesByCategory(
    EECInventoryItemCategory Category) const
{
    TArray<FInventoryEntry> FilteredEntries;
    UECInventoryManagerComponent* Inventory = InventoryComponent.Get();
    if (!Inventory)
    {
        return FilteredEntries;
    }

    const TArray<FInventoryEntry>& Entries = GetInventoryEntries();
    FilteredEntries.Reserve(Entries.Num());

    UECItemDataSubsystem& ItemDataSubsystem = UECItemDataSubsystem::Get(Inventory);
    for (const FInventoryEntry& Entry : Entries)
    {
        if (!IsValid(Entry.Instance) || Entry.StackCount <= 0)
        {
            continue;
        }

        const FECItemTableRow* ItemData = ItemDataSubsystem.FindItemData(Entry.Instance);
        const EECInventoryItemCategory ItemCategory = ItemData
            ? ItemData->InventoryCategory
            : EECInventoryItemCategory::Item;

        if (ItemCategory == Category)
        {
            FilteredEntries.Add(Entry);
        }
    }

    return FilteredEntries;
}

bool UInventoryViewModel::ContainsItem(UECInventoryItemInstance* ItemInstance) const
{
    if (!IsValid(ItemInstance))
    {
        return false;
    }

    return GetInventoryEntries().ContainsByPredicate(
        [ItemInstance](const FInventoryEntry& Entry)
        {
            return Entry.Instance == ItemInstance && Entry.StackCount > 0;
        });
}

bool UInventoryViewModel::HasInventory() const
{
    return InventoryComponent.IsValid();
}

void UInventoryViewModel::OnInventoryUpdated()
{
    OnVMInventoryUpdated.Broadcast();
}

void UInventoryViewModel::BeginDestroy()
{
    if (UECInventoryManagerComponent* Inventory = InventoryComponent.Get())
    {
        Inventory->OnInventoryUpdated.RemoveAll(this);
    }

    InventoryComponent.Reset();

    Super::BeginDestroy();
}
