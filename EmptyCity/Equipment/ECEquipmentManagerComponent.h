#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "AbilitySystem/ECAbilitySet.h"
#include "ECEquipmentManagerComponent.generated.h"

class UECAbilitySystemComponent;
class UECEquipmentInstance;
class UECEquipmentDefinition;

/**
 * 현재 장착 중인 장비 하나를 표현하는 엔트리입니다.
 * "무엇을(Definition) 장착했고, 그 결과 생성된 실물(Instance)과 부여된 어빌리티(Handles)가 무엇인지"를 한 묶음으로 보관합니다.
 */
USTRUCT()
struct FAppliedEquipmentEntry
{
	GENERATED_BODY()

	/** 이 장착의 원본이 된 설계도(에셋)입니다. */
	UPROPERTY()
	TSubclassOf<UECEquipmentDefinition> EquipmentDefinition;

	/** 설계도를 바탕으로 장착 시 생성된 런타임 인스턴스(실물)입니다. */
	UPROPERTY()
	TObjectPtr<UECEquipmentInstance> Instance = nullptr;

	/** 이 장비가 부여한 어빌리티들의 핸들입니다. 해제 시 이 핸들로 어빌리티를 한 번에 회수합니다. */
	UPROPERTY()
	FAbilitySet_GrantedHandles GrantedHandles;
};

/**
 * 현재 장착 중인 장비 엔트리들을 담는 컨테이너입니다.
 * 단순 배열로 둘 수도 있지만, 추가/제거 시 어빌리티 부여·회수와 Actor 스폰·정리를 함께 처리해야 하므로
 * AddEntry/RemoveEntry로 그 로직을 한곳에 모아 둔 별도 구조체로 분리했습니다.
 */
USTRUCT(BlueprintType)
struct FEquipmentList
{
	GENERATED_BODY()

	FEquipmentList(UActorComponent* InOwnerComponent = nullptr)
		: OwnerComponent(InOwnerComponent)
	{}

// ─────────────────────────────────────────────────────────────
// Add / Remove
// ─────────────────────────────────────────────────────────────
	/**
	 * 새 장비를 목록에 추가합니다.
	 * 인스턴스 생성 → AbilitySet 부여 → 장비 Actor 스폰까지 수행하고, 생성된 인스턴스를 반환합니다.
	 */
	UECEquipmentInstance* AddEntry(TSubclassOf<UECEquipmentDefinition> EquipmentDefinition);

	/**
	 * 해당 인스턴스에 대응하는 엔트리를 목록에서 제거합니다.
	 * 부여했던 어빌리티 회수 → 스폰한 Actor 정리 → 엔트리 삭제 순으로 처리합니다.
	 */
	void RemoveEntry(UECEquipmentInstance* Instance);

// ─────────────────────────────────────────────────────────────
// Helper
// ─────────────────────────────────────────────────────────────
	/** 소유 Actor에 붙어 있는 AbilitySystemComponent를 찾아 반환합니다. (어빌리티 부여/회수 대상) */
	UECAbilitySystemComponent* GetAbilitySystemComponent() const;

// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
	/** 현재 장착 중인 장비 엔트리 목록입니다. */
	UPROPERTY()
	TArray<FAppliedEquipmentEntry> Entries;

	/** 이 리스트를 소유한 컴포넌트입니다. 소유 Actor와 권한(Authority)을 알아내는 기준점이 됩니다. */
	UPROPERTY()
	TObjectPtr<UActorComponent> OwnerComponent;
};

/**
 * Pawn에 직접 붙어, 그 Pawn이 장착한 장비의 생성/제거를 관리하는 컴포넌트입니다.
 *
 * [InventoryManagerComponent와의 역할 분리]
 * - InventoryManager(PlayerController에 부착) : 플레이어가 "보유한" 아이템 목록을 관리합니다.
 * - EquipmentManager(이 클래스, Pawn에 부착) : Pawn이 실제로 "장착한" 장비를 관리합니다. (Actor 스폰, 어빌리티 부여 등)
 *
 * UPawnComponent를 상속하므로 Pawn에 직접 접근할 수 있고, 덕분에 Pawn의 Mesh 소켓에 장비 Actor를 붙일 수 있습니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EMPTYCITY_API UECEquipmentManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UECEquipmentManagerComponent(const FObjectInitializer& Initializer = FObjectInitializer::Get());

// ─────────────────────────────────────────────────────────────
// Equip / Unequip
// ─────────────────────────────────────────────────────────────
public:
	/** 설계도를 받아 장비를 장착하고, 생성된 인스턴스를 반환합니다. (내부에서 EquipmentList에 엔트리 추가 후 OnEquipped 발행) */
	UECEquipmentInstance* EquipItem(TSubclassOf<UECEquipmentDefinition> EquipmentDefinition);

	/** 장착된 인스턴스를 해제합니다. (OnUnequipped 발행 후 EquipmentList에서 엔트리 제거) */
	void UnequipItem(UECEquipmentInstance* ItemInstance);

// ─────────────────────────────────────────────────────────────
// Query
// ─────────────────────────────────────────────────────────────
public:
	/** 현재 장착 중인 장비 중 지정한 타입(또는 그 파생 타입)인 인스턴스만 모아 반환합니다. */
	UFUNCTION(BlueprintCallable)
	TArray<UECEquipmentInstance*> GetEquipmentInstancesOfType(TSubclassOf<UECEquipmentInstance> InstanceType) const;

// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
public:
	/** 이 Pawn이 현재 장착 중인 장비 목록입니다. */
	UPROPERTY()
	FEquipmentList EquipmentList;

};
