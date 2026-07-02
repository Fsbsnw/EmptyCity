#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ECInventoryManagerComponent.generated.h"

/** 인벤토리에 아이템이 추가될 때 호출하는 델리게이트입니다. */
DECLARE_MULTICAST_DELEGATE(FOnInventoryUpdatedSignature);

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
	/** 설계도와 개수를 받아 아이템 인스턴스를 생성하고 목록에 추가한 뒤, 생성된 인스턴스를 반환합니다. */
	UECInventoryItemInstance* AddEntry(TSubclassOf<UECInventoryItemDefinition> ItemDef, int32 StackCount);

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

// ─────────────────────────────────────────────────────────────
// Add / Remove
// ─────────────────────────────────────────────────────────────
public:
	/** 설계도와 개수를 받아 인벤토리에 아이템을 추가하고, 생성된 인스턴스를 반환합니다. */
	UECInventoryItemInstance* AddItemDefinition(TSubclassOf<UECInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	/** 지정한 아이템 인스턴스를 인벤토리에서 제거합니다. */
	void RemoveItemInstance(UECInventoryItemInstance* ItemInstance);

	/** 해당 설계도의 아이템을 인벤토리에 추가할 수 있는지 검사합니다. (최대 보유량·중복 제한 등의 판정 지점) */
	bool CanAddItemDefinition(TSubclassOf<UECInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	/** 인벤토리에 아이템이 추가될 때 호출하는 델리게이트입니다. */
	FOnInventoryUpdatedSignature OnInventoryUpdated;

// ─────────────────────────────────────────────────────────────
// Consume
// ─────────────────────────────────────────────────────────────
public:
	/**
	 * 해당 설계도의 아이템을 NumToConsume개만큼 소모합니다.
	 * 보유 수량이 부족하면 더이상 소모하지 않고 false를 반환합니다.
	 */
	bool ConsumeItemsByDefinition(TSubclassOf<UECInventoryItemDefinition> ItemDef, int32 NumToConsume);
	

// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
public:
	/** 플레이어가 보유한 아이템 목록입니다. */
	UPROPERTY()
	FInventoryList InventoryList;
};
