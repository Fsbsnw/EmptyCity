#include "ECInventoryManagerComponent.h"
#include "Data/Item/ECItemTableRow.h"
#include "ECInventoryItemDefinition.h"
#include "ECInventoryItemInstance.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Misc/ScopeExit.h"
#include "Save/ECSaveSubsystem.h"
#include "Subsystem/ECGameplayMessageSubsystem.h"
#include "Subsystem/ECItemDataSubsystem.h"
#include "Subsystem/ECMessageTypes.h"

namespace
{
	const FECItemTableRow* FindItemData(
		const UECInventoryManagerComponent* Inventory,
		TSubclassOf<UECInventoryItemDefinition> ItemDef)
	{
		return Inventory && ItemDef
			? UECItemDataSubsystem::Get(Inventory).FindItemData(ItemDef)
			: nullptr;
	}

	bool IsPlayerOwnedPermanentItem(
		const UECInventoryManagerComponent* Inventory,
		TSubclassOf<UECInventoryItemDefinition> ItemDef)
	{
		const FECItemTableRow* ItemData = FindItemData(Inventory, ItemDef);
		return Inventory
			&& Cast<APlayerController>(Inventory->GetOwner())
			&& ItemData
			&& ItemData->bPermanentPlayerItem;
	}

	float ResolveItemUnitWeight(
		const UECInventoryManagerComponent* Inventory,
		const FECItemTableRow* ItemData)
	{
		if (!Inventory || !ItemData)
		{
			return 0.0f;
		}

		const float BaseWeight = FMath::Max(0.0f, ItemData->Weight);
		if (ItemData->ToolUpgradeType != EECWorkbenchUpgradeType::Axe
			&& ItemData->ToolUpgradeType != EECWorkbenchUpgradeType::Pickaxe
			&& ItemData->ToolUpgradeType != EECWorkbenchUpgradeType::Weapon)
		{
			return BaseWeight;
		}

		const APlayerController* PlayerController = Cast<APlayerController>(Inventory->GetOwner());
		const AECPlayerState* PlayerState = PlayerController
			? PlayerController->GetPlayerState<AECPlayerState>()
			: nullptr;
		float UpgradedWeight = 0.0f;
		return PlayerState
			&& PlayerState->GetWorkbenchUpgradeValue(ItemData->ToolUpgradeType, TEXT("Weight"), UpgradedWeight)
			? FMath::Max(0.0f, UpgradedWeight)
			: BaseWeight;
	}

	bool BuildAggregatedRequirements(
		const TArray<FECInventoryItemRequirement>& Requirements,
		TMap<TSubclassOf<UECInventoryItemDefinition>, int32>& OutAggregatedRequirements)
	{
		OutAggregatedRequirements.Reset();

		for (const FECInventoryItemRequirement& Requirement : Requirements)
		{
			if (!Requirement.ItemDefinition || Requirement.Count <= 0)
			{
				OutAggregatedRequirements.Reset();
				return false;
			}

			int32& AggregatedCount = OutAggregatedRequirements.FindOrAdd(Requirement.ItemDefinition);
			if (Requirement.Count > MAX_int32 - AggregatedCount)
			{
				OutAggregatedRequirements.Reset();
				return false;
			}

			AggregatedCount += Requirement.Count;
		}

		return true;
	}
}


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

UECInventoryItemInstance* UECInventoryManagerComponent::AddItemDefinition(
	TSubclassOf<UECInventoryItemDefinition> ItemDef,
	const int32 StackCount,
	const bool bNotifyItemAcquired)
{
	const FECItemTableRow* ItemData = UECItemDataSubsystem::Get(this).FindItemData(ItemDef);
	if (!ItemData) return nullptr;

	// 영구 장비는 Pawn이 다시 생성돼 초기 아이템 지급이 재호출되어도 한 개만 유지합니다.
	if (IsPlayerOwnedPermanentItem(this, ItemDef))
	{
		const FInventoryEntry* ExistingEntry = InventoryList.Entries.FindByPredicate(
			[ItemDef](const FInventoryEntry& Entry)
			{
				return IsValid(Entry.Instance)
					&& Entry.Instance->ItemDef == ItemDef
					&& Entry.StackCount > 0;
			});
		if (ExistingEntry)
		{
			return ExistingEntry->Instance;
		}
	}

	if (!CanAddItemDefinition(ItemDef, StackCount)) return nullptr;

	UECInventoryItemInstance* Result = InventoryList.AddEntry(ItemDef, StackCount, ItemData->StackMax, MaxInventorySlots);
	if (!Result) return nullptr;
	OnInventoryUpdated.Broadcast();

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC || !bNotifyItemAcquired) return Result;

	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UECSaveSubsystem* SaveSubsystem = GameInstance
		? GameInstance->GetSubsystem<UECSaveSubsystem>()
		: nullptr;
	if (SaveSubsystem && SaveSubsystem->IsLoadInProgress()) return Result;
		
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
	if (!IsValid(ItemInstance)
		|| IsPlayerOwnedPermanentItem(this, ItemInstance->ItemDef))
	{
		return;
	}

	InventoryList.RemoveEntry(ItemInstance);
	OnInventoryUpdated.Broadcast();
}

bool UECInventoryManagerComponent::CanAddItemDefinition(TSubclassOf<UECInventoryItemDefinition> ItemDef, int32 StackCount) const
{
	return StackCount > 0 && GetMaxAddableItemCount(ItemDef, StackCount) >= StackCount;
}

int32 UECInventoryManagerComponent::GetMaxAddableItemCount(
	TSubclassOf<UECInventoryItemDefinition> ItemDef,
	const int32 RequestedCount) const
{
	if (!ItemDef || RequestedCount <= 0 || !GetWorld())
	{
		return 0;
	}

	const FECItemTableRow* ItemData = UECItemDataSubsystem::Get(this).FindItemData(ItemDef);
	if (!ItemData)
	{
		return 0;
	}

	if (IsPlayerOwnedPermanentItem(this, ItemDef))
	{
		const bool bAlreadyOwned = InventoryList.Entries.ContainsByPredicate(
			[ItemDef](const FInventoryEntry& Entry)
			{
				return IsValid(Entry.Instance)
					&& Entry.Instance->ItemDef == ItemDef
					&& Entry.StackCount > 0;
			});
		if (bAlreadyOwned)
		{
			return 0;
		}
	}

	const int32 StackMax = FMath::Max(1, ItemData->StackMax);
	int64 AvailableCapacity = static_cast<int64>(FMath::Max(0, MaxInventorySlots - InventoryList.Entries.Num())) * StackMax;
	for (const FInventoryEntry& Entry : InventoryList.Entries)
	{
		if (IsValid(Entry.Instance) && Entry.Instance->ItemDef == ItemDef && Entry.StackCount < StackMax)
		{
			AvailableCapacity += StackMax - FMath::Max(0, Entry.StackCount);
		}
	}

	int64 AddableCount = FMath::Min<int64>(RequestedCount, AvailableCapacity);
	if (IsPlayerOwnedPermanentItem(this, ItemDef))
	{
		AddableCount = FMath::Min<int64>(AddableCount, 1);
	}

	const double UnitWeight = static_cast<double>(ResolveItemUnitWeight(this, ItemData));
	if (MaxInventoryWeight > 0.0f && UnitWeight > UE_SMALL_NUMBER)
	{
		const double RemainingWeight = static_cast<double>(MaxInventoryWeight)
			- static_cast<double>(GetTotalWeight());
		const int64 AddableByWeight = RemainingWeight > -UE_KINDA_SMALL_NUMBER
			? FMath::Max<int64>(0, FMath::FloorToInt64(
				(RemainingWeight + UE_KINDA_SMALL_NUMBER) / UnitWeight))
			: 0;
		AddableCount = FMath::Min(AddableCount, AddableByWeight);
	}

	return static_cast<int32>(FMath::Clamp<int64>(AddableCount, 0, RequestedCount));
}

int32 UECInventoryManagerComponent::GetItemCountByDefinition(TSubclassOf<UECInventoryItemDefinition> ItemDef) const
{
	if (!ItemDef)
	{
		return 0;
	}

	int64 TotalCount = 0;
	for (const FInventoryEntry& Entry : InventoryList.Entries)
	{
		if (IsValid(Entry.Instance) && Entry.Instance->ItemDef == ItemDef)
		{
			TotalCount += static_cast<int64>(FMath::Max(0, Entry.StackCount));
			if (TotalCount >= MAX_int32)
			{
				return MAX_int32;
			}
		}
	}

	return static_cast<int32>(TotalCount);
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
		const FECItemTableRow* ItemData = FindItemData(this, ItemDef);
		if (!ItemData
			|| !ItemData->bCanTransfer
			|| IsPlayerOwnedPermanentItem(this, ItemDef))
		{
			continue;
		}

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

	const FECItemTableRow* ItemData = FindItemData(this, ItemInstance->ItemDef);
	if (!ItemData
		|| !ItemData->bCanTransfer
		|| IsPlayerOwnedPermanentItem(this, ItemInstance->ItemDef))
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

	const FECItemTableRow* ItemData = FindItemData(this, ItemInstance->ItemDef);
	if (!ItemData
		|| !ItemData->bCanDiscard
		|| IsPlayerOwnedPermanentItem(this, ItemInstance->ItemDef))
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
		if (!IsValid(Entry.Instance) || Entry.StackCount <= 0)
		{
			continue;
		}

		const FECItemTableRow* ItemData = ItemDataSubsystem.FindItemData(Entry.Instance);
		if (ItemData)
		{
			TotalWeight += ResolveItemUnitWeight(this, ItemData) * Entry.StackCount;
		}
	}
	return TotalWeight;
}

float UECInventoryManagerComponent::GetItemUnitWeight(const UECInventoryItemInstance* ItemInstance) const
{
	if (!IsValid(ItemInstance))
	{
		return 0.0f;
	}

	const FECItemTableRow* ItemData = UECItemDataSubsystem::Get(this).FindItemData(ItemInstance);
	return ResolveItemUnitWeight(this, ItemData);
}

float UECInventoryManagerComponent::GetItemDefinitionUnitWeight(
	TSubclassOf<UECInventoryItemDefinition> ItemDef) const
{
	return ResolveItemUnitWeight(this, FindItemData(this, ItemDef));
}

void UECInventoryManagerComponent::SetMaxInventoryWeight(float NewMaxInventoryWeight)
{
	const float ClampedWeight = FMath::Max(0.0f, NewMaxInventoryWeight);
	if (FMath::IsNearlyEqual(MaxInventoryWeight, ClampedWeight))
	{
		return;
	}

	MaxInventoryWeight = ClampedWeight;
	OnMaxInventoryWeightChanged.Broadcast(MaxInventoryWeight);
}

bool UECInventoryManagerComponent::ConsumeItemsByDefinition(TSubclassOf<UECInventoryItemDefinition> ItemDef, int32 NumToConsume)
{
	FECInventoryItemRequirement Requirement;
	Requirement.ItemDefinition = ItemDef;
	Requirement.Count = NumToConsume;
	return ConsumeItemRequirements({ Requirement });
}

bool UECInventoryManagerComponent::CanConsumeItemRequirements(
	const TArray<FECInventoryItemRequirement>& Requirements) const
{
	if (!GetOwner())
	{
		return false;
	}

	TMap<TSubclassOf<UECInventoryItemDefinition>, int32> AggregatedRequirements;
	if (!BuildAggregatedRequirements(Requirements, AggregatedRequirements))
	{
		return false;
	}

	for (const TPair<TSubclassOf<UECInventoryItemDefinition>, int32>& Requirement : AggregatedRequirements)
	{
		if (IsPlayerOwnedPermanentItem(this, Requirement.Key))
		{
			return false;
		}

		if (GetItemCountByDefinition(Requirement.Key) < Requirement.Value)
		{
			return false;
		}
	}

	return true;
}

bool UECInventoryManagerComponent::ConsumeItemRequirements(
	const TArray<FECInventoryItemRequirement>& Requirements)
{
	return ConsumeItemRequirementsWithCommit(Requirements, []() {});
}

bool UECInventoryManagerComponent::ConsumeItemRequirementsWithCommit(
	const TArray<FECInventoryItemRequirement>& Requirements,
	TFunctionRef<void()> CommitCallback)
{
	if (!GetOwner() || bRequirementTransactionInProgress)
	{
		return false;
	}

	TMap<TSubclassOf<UECInventoryItemDefinition>, int32> AggregatedRequirements;
	if (!BuildAggregatedRequirements(Requirements, AggregatedRequirements))
	{
		return false;
	}

	// 변경 전에 모든 재료를 검사한다. 이후 구간은 실패 가능한 외부 호출 없이 직접 수량만 변경한다.
	for (const TPair<TSubclassOf<UECInventoryItemDefinition>, int32>& Requirement : AggregatedRequirements)
	{
		if (IsPlayerOwnedPermanentItem(this, Requirement.Key))
		{
			return false;
		}

		if (GetItemCountByDefinition(Requirement.Key) < Requirement.Value)
		{
			return false;
		}
	}

	if (AggregatedRequirements.IsEmpty())
	{
		TGuardValue<bool> TransactionGuard(bRequirementTransactionInProgress, true);
		CommitCallback();
		return true;
	}

	TGuardValue<bool> TransactionGuard(bRequirementTransactionInProgress, true);
	for (const TPair<TSubclassOf<UECInventoryItemDefinition>, int32>& Requirement : AggregatedRequirements)
	{
		int32 Remaining = Requirement.Value;
		for (auto EntryIt = InventoryList.Entries.CreateIterator(); EntryIt && Remaining > 0; ++EntryIt)
		{
			FInventoryEntry& Entry = *EntryIt;
			if (!IsValid(Entry.Instance) || Entry.Instance->ItemDef != Requirement.Key)
			{
				continue;
			}

			const int32 ConsumedFromEntry = FMath::Min(FMath::Max(0, Entry.StackCount), Remaining);
			Entry.StackCount -= ConsumedFromEntry;
			Remaining -= ConsumedFromEntry;

			if (Entry.StackCount <= 0)
			{
				EntryIt.RemoveCurrent();
			}
		}

		checkf(Remaining == 0, TEXT("재료 사전 검사 후 인벤토리 수량이 예기치 않게 변경되었습니다."));
	}

	CommitCallback();
	OnInventoryUpdated.Broadcast();
	return true;
}

bool UECInventoryManagerComponent::CanConsumeItemRequirementsFromInventories(
	const TArray<UECInventoryManagerComponent*>& Inventories,
	const TArray<FECInventoryItemRequirement>& Requirements)
{
	TArray<UECInventoryManagerComponent*> UniqueInventories;
	for (UECInventoryManagerComponent* Inventory : Inventories)
	{
		if (IsValid(Inventory) && Inventory->GetOwner())
		{
			UniqueInventories.AddUnique(Inventory);
		}
	}

	if (UniqueInventories.IsEmpty())
	{
		return false;
	}

	TMap<TSubclassOf<UECInventoryItemDefinition>, int32> AggregatedRequirements;
	if (!BuildAggregatedRequirements(Requirements, AggregatedRequirements))
	{
		return false;
	}

	for (const TPair<TSubclassOf<UECInventoryItemDefinition>, int32>& Requirement : AggregatedRequirements)
	{
		int64 AvailableCount = 0;
		for (const UECInventoryManagerComponent* Inventory : UniqueInventories)
		{
			if (IsPlayerOwnedPermanentItem(Inventory, Requirement.Key))
			{
				return false;
			}

			AvailableCount += Inventory->GetItemCountByDefinition(Requirement.Key);
			if (AvailableCount >= Requirement.Value)
			{
				break;
			}
		}

		if (AvailableCount < Requirement.Value)
		{
			return false;
		}
	}

	return true;
}

bool UECInventoryManagerComponent::ConsumeItemRequirementsFromInventoriesWithCommit(
	const TArray<UECInventoryManagerComponent*>& Inventories,
	const TArray<FECInventoryItemRequirement>& Requirements,
	TFunctionRef<void()> CommitCallback)
{
	TArray<UECInventoryManagerComponent*> UniqueInventories;
	for (UECInventoryManagerComponent* Inventory : Inventories)
	{
		if (IsValid(Inventory) && Inventory->GetOwner())
		{
			UniqueInventories.AddUnique(Inventory);
		}
	}

	if (UniqueInventories.IsEmpty())
	{
		return false;
	}

	for (const UECInventoryManagerComponent* Inventory : UniqueInventories)
	{
		if (Inventory->bRequirementTransactionInProgress)
		{
			return false;
		}
	}

	TMap<TSubclassOf<UECInventoryItemDefinition>, int32> AggregatedRequirements;
	if (!BuildAggregatedRequirements(Requirements, AggregatedRequirements)
		|| !CanConsumeItemRequirementsFromInventories(UniqueInventories, Requirements))
	{
		return false;
	}

	for (UECInventoryManagerComponent* Inventory : UniqueInventories)
	{
		Inventory->bRequirementTransactionInProgress = true;
	}
	ON_SCOPE_EXIT
	{
		for (UECInventoryManagerComponent* Inventory : UniqueInventories)
		{
			Inventory->bRequirementTransactionInProgress = false;
		}
	};

	for (const TPair<TSubclassOf<UECInventoryItemDefinition>, int32>& Requirement : AggregatedRequirements)
	{
		int32 Remaining = Requirement.Value;
		for (UECInventoryManagerComponent* Inventory : UniqueInventories)
		{
			for (auto EntryIt = Inventory->InventoryList.Entries.CreateIterator(); EntryIt && Remaining > 0; ++EntryIt)
			{
				FInventoryEntry& Entry = *EntryIt;
				if (!IsValid(Entry.Instance) || Entry.Instance->ItemDef != Requirement.Key)
				{
					continue;
				}

				const int32 ConsumedFromEntry = FMath::Min(FMath::Max(0, Entry.StackCount), Remaining);
				Entry.StackCount -= ConsumedFromEntry;
				Remaining -= ConsumedFromEntry;

				if (Entry.StackCount <= 0)
				{
					EntryIt.RemoveCurrent();
				}
			}

			if (Remaining == 0)
			{
				break;
			}
		}

		checkf(Remaining == 0, TEXT("재료 사전 검사 후 통합 인벤토리 수량이 예기치 않게 변경되었습니다."));
	}

	CommitCallback();

	// 통합 재료를 관찰하는 UI가 어느 인벤토리에서 차감됐는지와 무관하게 갱신되도록 모두 알립니다.
	for (UECInventoryManagerComponent* Inventory : UniqueInventories)
	{
		Inventory->OnInventoryUpdated.Broadcast();
	}

	return true;
}
