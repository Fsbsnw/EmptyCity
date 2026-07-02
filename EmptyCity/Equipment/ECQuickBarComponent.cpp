#include "ECQuickBarComponent.h"
#include "ECEquipmentDefinition.h"
#include "ECEquipmentInstance.h"
#include "ECEquipmentManagerComponent.h"
#include "ECGameplayTags.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Inventory/Fragment/InventoryFragment_EquippableItem.h"
#include "Inventory/ECInventoryItemInstance.h"

void UECQuickBarComponent::BeginPlay()
{
	// Slots 배열을 NumSlots 크기로 미리 확장한다. (AddItemToSlot에서 IsValidIndex로 경계 검사)
	if (Slots.Num() < NumSlots)
	{
		Slots.AddDefaulted(NumSlots - Slots.Num());
	}

	Super::BeginPlay();
}

void UECQuickBarComponent::AddItemToSlot(int32 SlotIndex, UECInventoryItemInstance* Item)
{
	// 유효한 인덱스이고 해당 슬롯이 비어 있을 때만 등록한다. (슬롯 덮어쓰기 방지)
	if (Slots.IsValidIndex(SlotIndex) && (Item != nullptr))
	{
		if (Slots[SlotIndex] == nullptr)
		{
			Slots[SlotIndex] = Item;
		}
	}
}

void UECQuickBarComponent::SetActiveSlotIndex(int32 NewIndex)
{
	if (Slots.IsValidIndex(NewIndex) && (ActiveSlotIndex != NewIndex))
	{
		// 이전 슬롯 장비를 해제한 뒤 인덱스를 갱신하고 새 슬롯 장비를 장착한다.
		UnequipItemInSlot();
		ActiveSlotIndex = NewIndex;
		EquipItemInSlot();
	}
}

UECInventoryItemInstance* UECQuickBarComponent::RemoveItemFromSlot(int32 SlotIndex)
{
	UECInventoryItemInstance* Result = nullptr;

	// 제거할 슬롯이 현재 활성 슬롯이면, 장비를 먼저 해제하고 활성 상태를 해제(-1)한다.
	if (ActiveSlotIndex == SlotIndex)
	{
		UnequipItemInSlot();
		ActiveSlotIndex = -1;
	}

	if (Slots.IsValidIndex(SlotIndex))
	{
		// 슬롯에 등록돼 있던 아이템을 반환값으로 넘기고 슬롯은 빈 상태로 되돌린다.
		Result = Slots[SlotIndex];

		if (Result != nullptr)
		{
			Slots[SlotIndex] = nullptr;
		}
	}

	return Result;
}

UECInventoryItemInstance* UECQuickBarComponent::GetActiveSlotItem() const
{
	return Slots.IsValidIndex(ActiveSlotIndex) ? Slots[ActiveSlotIndex] : nullptr;
}

void UECQuickBarComponent::UseSlot(int32 SlotIndex)
{
	// 빈 슬롯이거나 인덱스가 유효하지 않으면 보낼 아이템이 없다.
	if (!Slots.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%hs] : 유효하지 않은 QuickBar 슬롯 인덱스: %d"), __FUNCTION__, SlotIndex);
		return;
	}

	UECInventoryItemInstance* Item = Slots[SlotIndex];
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%hs] : QuickBar 슬롯 %d 가 비어 있습니다."), __FUNCTION__, SlotIndex);
		return;
	}

	// GA_UseItem은 PlayerState의 ASC에 부여되어 있으므로, 그 ASC로 이벤트를 보낸다.
	const AController* OwnerController = Cast<AController>(GetOwner());
	const AECPlayerState* PS = OwnerController ? OwnerController->GetPlayerState<AECPlayerState>() : nullptr;
	UECAbilitySystemComponent* ASC = PS ? PS->GetAbilitySystemComponent() : nullptr;
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%hs] : ASC를 찾지 못해 슬롯 %d 사용 이벤트를 보내지 못했습니다."), __FUNCTION__, SlotIndex);
		return;
	}

	// "어떤 아이템을 쓸지"를 payload(OptionalObject)에 실어 보낸다. (인벤토리 UI 등 다른 진입점도 동일 이벤트를 사용)
	UE_LOG(LogTemp, Warning, TEXT("[%hs] : 슬롯 %d 아이템 사용 이벤트 발송: %s"), __FUNCTION__, SlotIndex, *Item->GetName());
	FGameplayEventData Payload;
	Payload.EventTag = ECGameplayTags::GameplayEvent_Item_Use;
	Payload.OptionalObject = Item;
	ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
}

void UECQuickBarComponent::UnequipItemInSlot()
{
	if (UECEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
	{
		if (EquippedItem)
		{
			EquipmentManager->UnequipItem(EquippedItem);
			EquippedItem = nullptr;
		}
	}
}

void UECQuickBarComponent::EquipItemInSlot()
{
	check(Slots.IsValidIndex(ActiveSlotIndex));
	check(EquippedItem == nullptr);

	if (UECInventoryItemInstance* SlotItem = Slots[ActiveSlotIndex])
	{
		// EquippableItem Fragment가 존재해야 장착 가능한 아이템이다.
		if (const UInventoryFragment_EquippableItem* EquipInfo = SlotItem->FindFragmentByClass<UInventoryFragment_EquippableItem>())
		{
			if (const TSubclassOf<UECEquipmentDefinition> EquipDef = EquipInfo->EquipmentDefinition)
			{
				if (UECEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
				{
					EquippedItem = EquipmentManager->EquipItem(EquipDef);
					if (EquippedItem)
					{
						// Instigator를 설정해 어느 InventoryItemInstance가 이 장착을 유발했는지 역추적할 수 있게 한다.
						EquippedItem->Instigator = SlotItem;
					}
				}
			}
		}
	}
}

UECEquipmentManagerComponent* UECQuickBarComponent::FindEquipmentManager() const
{
	// QuickBar는 Controller 소유이므로, 빙의된 Pawn에서 EquipmentManager를 찾는다.
	if (const AController* OwnerController = Cast<AController>(GetOwner()))
	{
		if (const APawn* Pawn = OwnerController->GetPawn())
		{
			return Pawn->FindComponentByClass<UECEquipmentManagerComponent>();
		}
	}
	return nullptr;
}
