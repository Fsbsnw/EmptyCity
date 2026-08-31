#include "ECInventoryManagerComponent.h"
#include "Data/Item/ECItemTableRow.h"
#include "ECInventoryItemDefinition.h"
#include "ECInventoryItemInstance.h"
#include "Engine/Texture2D.h"
#include "Subsystem/ECGameplayMessageSubsystem.h"
#include "Subsystem/ECItemDataSubsystem.h"
#include "Subsystem/ECMessageTypes.h"


UECInventoryItemInstance* FInventoryList::AddEntry(TSubclassOf<UECInventoryItemDefinition> ItemDef, int32 StackCount, int32 StackMax, int32 MaxSlots)
{
	check(ItemDef);
	check(OwnerComponent);
	if (StackCount <= 0 || MaxSlots <= 0) return nullptr;

	AActor* OwningActor = OwnerComponent->GetOwner();
	UECInventoryItemInstance* Result = nullptr;
	int32 Remaining = StackCount;
	StackMax = FMath::Max(1, StackMax);

	for (FInventoryEntry& Entry : Entries)
	{
		if (!IsValid(Entry.Instance) || Entry.Instance->ItemDef != ItemDef || Entry.StackCount >= StackMax) continue;
		const int32 AddedCount = FMath::Min(StackMax - Entry.StackCount, Remaining);
		Entry.StackCount += AddedCount;
		Remaining -= AddedCount;
		if (!Result) Result = Entry.Instance;
		if (Remaining == 0) return Result;
	}

	while (Remaining > 0 && Entries.Num() < MaxSlots)
	{
		FInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
		NewEntry.Instance = NewObject<UECInventoryItemInstance>(OwningActor);
		NewEntry.Instance->ItemDef = ItemDef;
		for (UECInventoryItemFragment* Fragment : GetDefault<UECInventoryItemDefinition>(ItemDef)->Fragments)
		{
			if (Fragment) Fragment->OnInstanceCreated(NewEntry.Instance);
		}
		NewEntry.StackCount = FMath::Min(StackMax, Remaining);
		Remaining -= NewEntry.StackCount;
		if (!Result) Result = NewEntry.Instance;
	}

	ensureMsgf(Remaining == 0, TEXT("CanAddItemDefinition 검사 이후 인벤토리 공간이 부족해졌습니다. 남은 수량: %d"), Remaining);
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

UECInventoryManagerComponent::UECInventoryManagerComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), InventoryList(this)
{
}

UECInventoryItemInstance* UECInventoryManagerComponent::AddItemDefinition(TSubclassOf<UECInventoryItemDefinition> ItemDef, int32 StackCount)
{
	if (!CanAddItemDefinition(ItemDef, StackCount)) return nullptr;
	const FECItemTableRow* ItemData = UECItemDataSubsystem::Get(this).FindItemData(ItemDef);
	if (!ItemData) return nullptr;

	UECInventoryItemInstance* Result = InventoryList.AddEntry(ItemDef, StackCount, ItemData->StackMax, MaxInventorySlots);
	if (!Result) return nullptr;
	OnInventoryUpdated.Broadcast();

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC) return Result;
		
	FECNotificationMessage Message;
	Message.TargetPlayer = PC->PlayerState;
	Message.TargetChannel = TAG_Notification_ItemAcquired;
	Message.PayloadTag = TAG_Notification_ItemAcquired_Normal;
	Message.PayloadMessage = ItemData->DisplayName;
	Message.PayloadValue = StackCount;
	if (UTexture2D* IconTexture = ItemData->Icon.LoadSynchronous())
	{
		Message.PayloadItemBrush.SetResourceObject(IconTexture);
	}
	UECGameplayMessageSubsystem& MessageSubsystem = UECGameplayMessageSubsystem::Get(this);
	MessageSubsystem.BroadcastMessage(TAG_AddNotification_Message, Message);
	return Result;
}

void UECInventoryManagerComponent::RemoveItemInstance(UECInventoryItemInstance* ItemInstance)
{
	InventoryList.RemoveEntry(ItemInstance);
	OnInventoryUpdated.Broadcast();
}

bool UECInventoryManagerComponent::CanAddItemDefinition(TSubclassOf<UECInventoryItemDefinition> ItemDef, int32 StackCount) const
{
	if (!ItemDef || StackCount <= 0 || !GetWorld()) return false;
	const FECItemTableRow* ItemData = UECItemDataSubsystem::Get(this).FindItemData(ItemDef);
	if (!ItemData) return false;

	const int32 StackMax = FMath::Max(1, ItemData->StackMax);
	int64 AvailableCapacity = static_cast<int64>(FMath::Max(0, MaxInventorySlots - InventoryList.Entries.Num())) * StackMax;
	for (const FInventoryEntry& Entry : InventoryList.Entries)
	{
		if (IsValid(Entry.Instance) && Entry.Instance->ItemDef == ItemDef && Entry.StackCount < StackMax) AvailableCapacity += StackMax - FMath::Max(0, Entry.StackCount);
	}

	return AvailableCapacity >= StackCount;
}

int32 UECInventoryManagerComponent::TransferAllItemsTo(UECInventoryManagerComponent* TargetInventory)
{
	if (!IsValid(TargetInventory) || TargetInventory == this)
	{
		return 0;
	}

	const TArray<FInventoryEntry> EntriesSnapshot =	InventoryList.Entries;

	int32 TotalTransferred = 0;

	for (const FInventoryEntry& SnapshotEntry : EntriesSnapshot)
	{
		if (!IsValid(SnapshotEntry.Instance) || SnapshotEntry.StackCount <= 0)
		{
			continue;
		}

		FInventoryEntry* SourceEntry =
			InventoryList.Entries.FindByPredicate(
				[&SnapshotEntry](const FInventoryEntry& Entry)
				{
					return Entry.Instance == SnapshotEntry.Instance;
				});

		if (!SourceEntry)
		{
			continue;
		}

		const TSubclassOf<UECInventoryItemDefinition> ItemDef =	SourceEntry->Instance->ItemDef;

		// 기존 CanAddItemDefinition만 사용해서
		// 들어갈 수 있는 최대 수량을 찾습니다.
		int32 Low = 0;
		int32 High = SourceEntry->StackCount;

		while (Low < High)
		{
			const int32 Middle = (Low + High + 1) / 2;

			if (TargetInventory->CanAddItemDefinition(ItemDef, Middle))
			{
				Low = Middle;
			}
			else
			{
				High = Middle - 1;
			}
		}

		const int32 TransferCount = Low;

		if (TransferCount <= 0)
		{
			continue;
		}

		if (!TargetInventory->AddItemDefinition(ItemDef, TransferCount))
		{
			continue;
		}

		SourceEntry->StackCount -= TransferCount;
		TotalTransferred += TransferCount;

		if (SourceEntry->StackCount <= 0)
		{
			InventoryList.RemoveEntry(SourceEntry->Instance);
		}
	}

	if (TotalTransferred > 0)
	{
		OnInventoryUpdated.Broadcast();
	}

	return TotalTransferred;
}

int32 UECInventoryManagerComponent::TransferItemTo(UECInventoryManagerComponent* TargetInventory, UECInventoryItemInstance* ItemInstance, int32 Count)
{
	if (!IsValid(TargetInventory) || TargetInventory == this || !IsValid(ItemInstance) || Count <= 0)
	{
		return 0;
	}

	FInventoryEntry* SourceEntry =
		InventoryList.Entries.FindByPredicate(
			[ItemInstance](const FInventoryEntry& Entry)
			{
				return Entry.Instance == ItemInstance;
			});

	if (!SourceEntry || SourceEntry->StackCount <= 0)
	{
		return 0;
	}

	const int32 TransferCount =	FMath::Min(Count, SourceEntry->StackCount);

	if (!TargetInventory->CanAddItemDefinition(ItemInstance->ItemDef, TransferCount))
	{
		return 0;
	}

	if (!TargetInventory->AddItemDefinition(ItemInstance->ItemDef, TransferCount))
	{
		return 0;
	}

	SourceEntry->StackCount -= TransferCount;

	if (SourceEntry->StackCount <= 0)
	{
		InventoryList.RemoveEntry(ItemInstance);
	}

	OnInventoryUpdated.Broadcast();
	return TransferCount;
}

int32 UECInventoryManagerComponent::RemoveItemCount(UECInventoryItemInstance* ItemInstance, int32 Count)
{
	if (!IsValid(ItemInstance) || Count <= 0)
	{
		return 0;
	}

	FInventoryEntry* Entry = InventoryList.Entries.FindByPredicate(
		[ItemInstance](const FInventoryEntry& Candidate)
		{
			return Candidate.Instance == ItemInstance;
		});

	if (!Entry || Entry->StackCount <= 0)
	{
		return 0;
	}

	const int32 RemovedCount = FMath::Min(Count, Entry->StackCount);
	Entry->StackCount -= RemovedCount;

	if (Entry->StackCount <= 0)
	{
		InventoryList.RemoveEntry(ItemInstance);
	}

	OnInventoryUpdated.Broadcast();
	return RemovedCount;
}

float UECInventoryManagerComponent::GetTotalWeight() const
{
	float TotalWeight = 0.0f;
	UECItemDataSubsystem& ItemDataSubsystem = UECItemDataSubsystem::Get(this);
	for (const FInventoryEntry& Entry : InventoryList.Entries)
	{
		const FECItemTableRow* ItemData = ItemDataSubsystem.FindItemData(Entry.Instance);
		TotalWeight += ItemData->Weight * Entry.StackCount;
	}
	return TotalWeight;
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
