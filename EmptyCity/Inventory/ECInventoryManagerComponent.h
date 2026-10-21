#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/ECInventoryItemRequirement.h"
#include "ECInventoryManagerComponent.generated.h"

/** 인벤토리에 아이템이 추가될 때 호출하는 델리게이트입니다. */
DECLARE_MULTICAST_DELEGATE(FOnInventoryUpdatedSignature);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMaxInventoryWeightChangedSignature, float);

class UECInventoryItemDefinition;
class UECInventoryItemInstance;

/** 인벤토리 목록의 슬롯 하나를 나타내는 엔트리입니다. */
USTRUCT(BlueprintType)
struct FInventoryEntry
{
	GENERATED_BODY()

	/** 이 슬롯이 담고 있는 아이템 인스턴스입니다. */
	UPROPERTY()
	TObjectPtr<UECInventoryItemInstance> Instance = nullptr;
	
	/** 아이템 개수를 나타내는 변수입니다. */
	UPROPERTY()
	int32 StackCount = 0;
};

/**
 * 인벤토리 항목 전체를 담는 컨테이너입니다.
 * 아이템을 추가할 때 인스턴스 생성·소유권 설정 등 처리가 필요하므로, 그 로직을 AddEntry에 모아 둔 별도 구조체로 분리했습니다.
 * OwnerComponent를 들고 있어 소유 Actor와 권한(Authority)을 알아낼 수 있습니다.
 */
USTRUCT(BlueprintType)
struct FInventoryList
{
	GENERATED_BODY()

	FInventoryList(UActorComponent* InOwnerComponent = nullptr) : OwnerComponent(InOwnerComponent)
	{}

// ─────────────────────────────────────────────────────────────
// Entry
// ─────────────────────────────────────────────────────────────
	/** 기존 스택을 먼저 채운 뒤 남은 수량을 새 슬롯으로 분할하고, 처음 변경된 아이템 인스턴스를 반환합니다. */
	UECInventoryItemInstance* AddEntry(TSubclassOf<UECInventoryItemDefinition> ItemDef, int32 StackCount, int32 StackMax, int32 MaxSlots);

	/** 지정한 아이템 인스턴스를 가진 엔트리를 목록에서 찾아 제거합니다. */
	void RemoveEntry(UECInventoryItemInstance* ItemInstance);
	
// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
	/** 현재 보유 중인 아이템 슬롯 목록입니다. */
	UPROPERTY()
	TArray<FInventoryEntry> Entries;

	/** 이 리스트를 소유한 컴포넌트입니다. 소유 Actor와 권한을 알아내는 기준점이 됩니다. */
	UPROPERTY()
	TObjectPtr<UActorComponent> OwnerComponent;
};

/**
 * PlayerController에 부착되어 플레이어의 인벤토리 전체를 관리하는 컴포넌트입니다.
 *
 * [왜 UControllerComponent가 아니라 UActorComponent를 상속하나?]
 * - Lyra 원본에서 Replication을 ActorComponent 수준에서 처리하기 위한 설계입니다.
 * - PlayerController 소유이므로 서버 권한(Authority)을 자연스럽게 확보합니다.
 *
 * [Equipment와의 관계]
 * - InventoryManager(이 클래스) : "무엇을 보유했는가" 목록을 다룹니다.
 * - 장착 흐름: InventoryManager 보유 → EquipmentManager가 실제 장착.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EMPTYCITY_API UECInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UECInventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	static constexpr int32 MaxInventorySlots = 20;

// ─────────────────────────────────────────────────────────────
// Add / Remove
// ─────────────────────────────────────────────────────────────
public:
	/**
	 * 설계도와 개수를 받아 인벤토리에 아이템을 추가하고, 생성된 인스턴스를 반환합니다.
	 * 초기 지급이나 저장 데이터 복원처럼 실제 획득이 아닌 경우 bNotifyItemAcquired를 false로 전달합니다.
	 */
	UECInventoryItemInstance* AddItemDefinition(
		TSubclassOf<UECInventoryItemDefinition> ItemDef,
		int32 StackCount = 1,
		bool bNotifyItemAcquired = true);

	/** 지정한 아이템 인스턴스를 인벤토리에서 제거합니다. */
	void RemoveItemInstance(UECInventoryItemInstance* ItemInstance);

	/** 해당 설계도의 아이템을 인벤토리에 추가할 수 있는지 검사합니다. (최대 보유량·중복 제한 등의 판정 지점) */
	bool CanAddItemDefinition(TSubclassOf<UECInventoryItemDefinition> ItemDef, int32 StackCount = 1) const;

	/** 무게와 슬롯 제한을 함께 고려해 요청 수량 중 실제로 추가 가능한 최대 개수를 반환합니다. */
	int32 GetMaxAddableItemCount(
		TSubclassOf<UECInventoryItemDefinition> ItemDef,
		int32 RequestedCount) const;

	/** 해당 설계도에 해당하는 모든 인벤토리 엔트리의 수량 합계를 반환합니다. */
	int32 GetItemCountByDefinition(TSubclassOf<UECInventoryItemDefinition> ItemDef) const;

	/** 인벤토리에 아이템의 상태가 변경될 때 호출하는 델리게이트입니다. */
	FOnInventoryUpdatedSignature OnInventoryUpdated;

	/** 대상 인벤토리로 모든 아이템을 이동시킵니다. */
	int32 TransferAllItemsTo(UECInventoryManagerComponent* TargetInventory);

	/** 대상 인벤토리로 아이템을 이동시킵니다. */
	int32 TransferItemTo(UECInventoryManagerComponent* TargetInventory, UECInventoryItemInstance* ItemInstance, int32 Count = 1);

	/** 지정한 아이템 인스턴스의 수량을 제거합니다. */
	int32 RemoveItemCount(UECInventoryItemInstance* ItemInstance, int32 Count = 1);
	
// ─────────────────────────────────────────────────────────────
// Weight
// ─────────────────────────────────────────────────────────────
public:
	/** 현재 인벤토리에 보관된 모든 아이템의 총 무게를 반환합니다. */
	float GetTotalWeight() const;

	/** 현재 플레이어의 장비 강화 단계까지 반영한 아이템 한 개의 무게를 반환합니다. */
	float GetItemUnitWeight(const UECInventoryItemInstance* ItemInstance) const;

	/** Item Definition 기준으로 장비 강화 단계까지 반영한 아이템 한 개의 무게를 반환합니다. */
	float GetItemDefinitionUnitWeight(TSubclassOf<UECInventoryItemDefinition> ItemDef) const;

	/** 0 이하는 무게 제한 없음, 양수는 추가 가능한 최대 총 무게입니다. */
	float GetMaxInventoryWeight() const { return MaxInventoryWeight; }
	void SetMaxInventoryWeight(float NewMaxInventoryWeight);

	FOnMaxInventoryWeightChangedSignature OnMaxInventoryWeightChanged;

	
// ─────────────────────────────────────────────────────────────
// Consume
// ─────────────────────────────────────────────────────────────
public:
	/**
	 * 해당 설계도의 아이템을 NumToConsume개만큼 소모합니다.
	 * 보유 수량이 부족하면 더이상 소모하지 않고 false를 반환합니다.
	 */
	bool ConsumeItemsByDefinition(TSubclassOf<UECInventoryItemDefinition> ItemDef, int32 NumToConsume);

	/**
	 * 여러 재료의 보유 여부를 한 번에 검사합니다.
	 * 같은 ItemDefinition은 합산하며, 잘못된 항목이 하나라도 있으면 false를 반환합니다.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Transaction")
	bool CanConsumeItemRequirements(const TArray<FECInventoryItemRequirement>& Requirements) const;

	/**
	 * 모든 재료를 먼저 검사한 뒤 한 번의 트랜잭션으로 소모합니다.
	 * 하나라도 부족하거나 잘못된 항목이 있으면 인벤토리를 전혀 변경하지 않습니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transaction")
	bool ConsumeItemRequirements(const TArray<FECInventoryItemRequirement>& Requirements);

	/**
	 * C++ 전용 트랜잭션 경로입니다. 재료 변경 후 CommitCallback을 실행하고 마지막에만 갱신 이벤트를 보냅니다.
	 * CommitCallback은 실패하지 않는 상태 커밋만 수행해야 합니다.
	 */
	bool ConsumeItemRequirementsWithCommit(
		const TArray<FECInventoryItemRequirement>& Requirements,
		TFunctionRef<void()> CommitCallback);

	/** 여러 인벤토리의 수량을 합산해 모든 재료를 지불할 수 있는지 검사합니다. */
	static bool CanConsumeItemRequirementsFromInventories(
		const TArray<UECInventoryManagerComponent*>& Inventories,
		const TArray<FECInventoryItemRequirement>& Requirements);

	/**
	 * 여러 인벤토리에서 앞쪽 인벤토리부터 재료를 소모한 뒤 상태를 커밋합니다.
	 * 사전 검사를 통과하지 못하면 어느 인벤토리도 변경하지 않습니다.
	 */
	static bool ConsumeItemRequirementsFromInventoriesWithCommit(
		const TArray<UECInventoryManagerComponent*>& Inventories,
		const TArray<FECInventoryItemRequirement>& Requirements,
		TFunctionRef<void()> CommitCallback);
	

// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
public:
	/** 플레이어가 보유한 아이템 목록입니다. */
	UPROPERTY()
	FInventoryList InventoryList;

private:
	/** 플레이어 강화 데이터에서 적용합니다. 보관함은 기본값 0으로 무제한입니다. */
	UPROPERTY(VisibleInstanceOnly, Category = "Inventory|Weight")
	float MaxInventoryWeight = 0.0f;

	/** 재료 소모의 커밋/알림 도중 중첩 트랜잭션을 차단합니다. */
	bool bRequirementTransactionInProgress = false;
};
