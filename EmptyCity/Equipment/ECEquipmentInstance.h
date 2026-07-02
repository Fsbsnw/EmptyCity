#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ECEquipmentInstance.generated.h"

struct FEquipmentActorToSpawn;

/**
 * 실제로 장착된 장비 하나를 나타내는 런타임 인스턴스입니다.
 *
 * [용어 정리]
 * - Definition(에셋) : 장비의 "설계도". 어떤 Actor를 어떤 소켓에 붙일지 등을 미리 정의해 둔 데이터입니다.
 * - Instance(이 클래스) : 설계도를 바탕으로 실제로 캐릭터에 장착된 "실물" 하나입니다.
 *
 * [생성/소멸 흐름]
 * 1. EquipmentManagerComponent가 Definition을 읽어 이 Instance를 NewObject로 생성합니다.
 * 2. SpawnEquipmentActors()로 무기/방어구 Actor를 캐릭터 소켓에 붙입니다.
 * 3. 장착 시 OnEquipped(), 해제 시 OnUnequipped()가 호출됩니다.
 * 4. 해제될 때 DestroyEquipmentActors()로 붙였던 Actor를 모두 제거합니다.
 *
 * 무기별 고유 동작(예: 발사, 재장전)이 필요하면 이 클래스를 상속(예: UECWeaponInstance)해 구현합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class EMPTYCITY_API UECEquipmentInstance : public UObject
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Equip / Unequip
// ─────────────────────────────────────────────────────────────
public:
	/**
	 * 장착이 끝난 직후 호출되는 진입점입니다. (EquipmentManager가 호출)
	 * 파생 클래스에서 오버라이드해 C++ 장착 로직을 추가할 수 있으며, 내부적으로 K2_OnEquipped를 발행해 Blueprint에도 알립니다.
	 */
	virtual void OnEquipped();

	/**
	 * 해제가 시작될 때 호출되는 진입점입니다. (EquipmentManager가 호출)
	 * 파생 클래스에서 오버라이드해 C++ 해제 로직을 추가할 수 있으며, 내부적으로 K2_OnUnequipped를 발행해 Blueprint에도 알립니다.
	 */
	virtual void OnUnequipped();
	

// ─────────────────────────────────────────────────────────────
// Pawn Accessors
// ─────────────────────────────────────────────────────────────
public:
	/**
	 * 이 장비를 소유한 Pawn(캐릭터)을 반환합니다.
	 * 생성 시 Pawn을 Outer로 지정했기 때문에, 별도 변수 없이 GetOuter()만으로 소유자를 알 수 있습니다.
	 */
	APawn* GetPawn() const;

	/**
	 * 소유 Pawn을 지정한 타입으로 캐스팅해 반환합니다. (타입이 맞지 않으면 nullptr)
	 * DeterminesOutputType 덕분에 Blueprint에서는 PawnType에 넣은 클래스가 그대로 반환 타입으로 표시됩니다.
	 */
	APawn* GetTypedPawn(TSubclassOf<APawn> PawnType) const;

	
// ─────────────────────────────────────────────────────────────
// Equipment Actors
// ─────────────────────────────────────────────────────────────
public:
	/**
	 * Definition의 ActorsToSpawn 목록을 읽어, 각 Actor를 월드에 스폰한 뒤 Pawn의 Mesh 소켓에 부착합니다.
	 * 스폰된 Actor는 나중에 일괄 제거할 수 있도록 SpawnedActors에 보관됩니다.
	 */
	void SpawnEquipmentActors(const TArray<FEquipmentActorToSpawn>& ActorsToSpawn);

	/** SpawnEquipmentActors로 스폰했던 모든 Actor를 월드에서 제거합니다. (해제 시 호출) */
	void DestroyEquipmentActors();

	/** 현재 스폰되어 부착되어 있는 장비 Actor 목록을 반환합니다. */
	TArray<AActor*> GetSpawnedActors() const { return SpawnedActors; }

	
// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
public:
	/**
	 * 이 장착을 요청한 InventoryItemInstance입니다.
	 * "어떤 인벤토리 아이템이 이 장비로 이어졌는지" 역추적할 때 사용하며, QuickBarComponent가 슬롯을 장착할 때 설정합니다.
	 */
	UPROPERTY()
	TObjectPtr<UObject> Instigator;

	/** SpawnEquipmentActors로 스폰된 Actor들을 보관합니다. DestroyEquipmentActors에서 이 목록을 순회해 일괄 제거합니다. */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedActors;
};
