#include "ECInventoryManagerComponent.h"
#include "ECInventoryItemDefinition.h"
#include "ECInventoryItemInstance.h"
#include "Fragment/InventoryFragment_ItemSlotIcon.h"
#include "Subsystem/ECGameplayMessageSubsystem.h"
#include "Subsystem/ECMessageTypes.h"


UECInventoryItemInstance* FInventoryList::AddEntry(TSubclassOf<UECInventoryItemDefinition> ItemDef, int32 StackCount)
{
	UECInventoryItemInstance* Result = nullptr;
	check(ItemDef);
	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();

	// OwningActor(PlayerController)를 Outer로 지정해 GC 수명을 Actor에 귀속시킵니다.
	FInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Instance = NewObject<UECInventoryItemInstance>(OwningActor);
	NewEntry.Instance->ItemDef = ItemDef;
	for (UECInventoryItemFragment* Fragment : GetDefault<UECInventoryItemDefinition>(ItemDef)->Fragments)
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(NewEntry.Instance);
		}
	}
	NewEntry.StackCount = StackCount;
	Result = NewEntry.Instance;
	
	return Result;
}

void FInventoryList::RemoveEntry(UECInventoryItemInstance* ItemInstance)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInventoryEntry& Entry = *EntryIt;
		if (Entry.Instance == ItemInstance)
		{
			EntryIt.RemoveCurrent();
		}
	}
}

UECInventoryManagerComponent::UECInventoryManagerComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
                                                                                                          , InventoryList(this)
{
}

UECInventoryItemInstance* UECInventoryManagerComponent::AddItemDefinition(TSubclassOf<UECInventoryItemDefinition> ItemDef, int32 StackCount)
{
	UECInventoryItemInstance* Result = nullptr;
	if (ItemDef)
	{
		Result = InventoryList.AddEntry(ItemDef, StackCount);
		OnInventoryUpdated.Broadcast();

		APlayerController* PC = Cast<APlayerController>(GetOwner());
		if (!PC)
		{
			return Result;
		}
		
		const UECInventoryItemDefinition* ItemDefCDO = ItemDef->GetDefaultObject<UECInventoryItemDefinition>();
		const UInventoryFragment_ItemSlotIcon* IconFragment =
			Cast<UInventoryFragment_ItemSlotIcon>(ItemDefCDO->FindFragmentByClass(UInventoryFragment_ItemSlotIcon::StaticClass()));
		
		FECNotificationMessage Message;
		Message.TargetPlayer = PC->PlayerState;
		Message.TargetChannel = TAG_Notification_ItemAcquired;
		Message.PayloadTag = TAG_Notification_ItemAcquired_Normal;
		Message.PayloadMessage = IconFragment->ItemName;
		Message.PayloadValue = StackCount;
		Message.PayloadItemBrush = IconFragment->ItemBrush;
		UECGameplayMessageSubsystem& MessageSubsystem = UECGameplayMessageSubsystem::Get(this);
		MessageSubsystem.BroadcastMessage(TAG_AddNotification_Message, Message);
	}
	return Result;
}

void UECInventoryManagerComponent::RemoveItemInstance(UECInventoryItemInstance* ItemInstance)
{
	InventoryList.RemoveEntry(ItemInstance);
	OnInventoryUpdated.Broadcast();
}

bool UECInventoryManagerComponent::CanAddItemDefinition(TSubclassOf<UECInventoryItemDefinition> ItemDef,
                                                        int32 StackCount)
{
	// @TODO: 아이템 최대 개수에 따라 제한 혹은 1개만 가질 수 있는 아이템 제한 등등
	return true;
}

bool UECInventoryManagerComponent::ConsumeItemsByDefinition(TSubclassOf<UECInventoryItemDefinition> ItemDef, int32 NumToConsume)
{
	if (!GetOwner() || NumToConsume <= 0)
	{
		return false;
	}

	// 먼저 보유 총량을 합산한다. 부족하면 아무것도 소모하지 않고 실패시킨다. (부분 소모 방지)
	int32 TotalAvailable = 0;
	for (const FInventoryEntry& Entry : InventoryList.Entries)
	{
		if (IsValid(Entry.Instance) && Entry.Instance->ItemDef == ItemDef)
		{
			TotalAvailable += Entry.StackCount;
		}
	}

	if (TotalAvailable < NumToConsume)
	{
		return false;
	}

	// StackCount를 줄여가며 소모하고, 0이 된 엔트리만 목록에서 제거한다. (스택 묶음을 통째로 날리지 않도록)
	int32 Remaining = NumToConsume;
	for (auto EntryIt = InventoryList.Entries.CreateIterator(); EntryIt && Remaining > 0; ++EntryIt)
	{
		FInventoryEntry& Entry = *EntryIt;
		if (!IsValid(Entry.Instance) || Entry.Instance->ItemDef != ItemDef)
		{
			continue;
		}

		const int32 ConsumedFromEntry = FMath::Min(Entry.StackCount, Remaining);
		Entry.StackCount -= ConsumedFromEntry;
		Remaining -= ConsumedFromEntry;

		if (Entry.StackCount <= 0)
		{
			EntryIt.RemoveCurrent();
		}
	}
	OnInventoryUpdated.Broadcast();

	return true;
}
