#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionOption.h"
#include "Inventory/IPickupable.h"
#include "ECPickupableItem.generated.h"

class UECInventoryItemDefinition;

/**
 * 월드에 떨어져 있어 플레이어가 다가가 주울 수 있는 아이템 액터입니다.
 *
 * 두 개의 인터페이스를 함께 구현합니다.
 * - IInteractableTarget : 상호작용 시스템에 "줍기" 선택지 하나를 제공합니다.
 * - IPickupable : 주울 때 인벤토리에 넘겨줄 내용물(StaticInventory)을 알려줍니다.
 *
 * 줍는 로직 자체는 이 액터가 갖지 않습니다. 옵션에 지정한 줍기 어빌리티(UECGameplayAbility_PickupItem)가
 * GetPickupInventory()로 내용물을 받아 인벤토리에 추가합니다.
 *
 * [감지되는 원리]
 * Mesh의 콜리전이 상호작용 채널(EC_TraceChannel_Interaction)에 응답해야
 * 주변 탐색(Overlap)과 정면 조준(LineTrace) 양쪽에서 상호작용 대상으로 잡힙니다.
 * 이를 위해 프로젝트에 정의된 "Interactable_BlockDynamic" 콜리전 프로파일을 사용합니다.
 */
UCLASS()
class EMPTYCITY_API AECPickupableItem : public AActor, public IInteractableTarget, public IPickupable
{
	GENERATED_BODY()

public:
	AECPickupableItem();

// ─────────────────────────────────────────────────────────────
// Interface
// ─────────────────────────────────────────────────────────────
public:
	/** IInteractableTarget : 이 아이템이 제공하는 "줍기" 상호작용 옵션을 수집기에 추가합니다. */
	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& InteractionBuilder) override;

	/** IPickupable : 주울 때 인벤토리에 넘겨줄 내용물(StaticInventory)을 반환합니다. */
	virtual FInventoryPickup GetPickupInventory() const override;

// ─────────────────────────────────────────────────────────────
// Component
// ─────────────────────────────────────────────────────────────
public:
	/** 월드에 보이는 외형이자 상호작용 콜리전입니다. (RootComponent로 사용) */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
protected:
	/** 플레이어에게 제공할 상호작용 선택지입니다. InteractionAbilityToGrant에 줍기 어빌리티를 지정해 두세요. */
	UPROPERTY(EditDefaultsOnly, Category = "변수")
	FInteractionOption Option;

	/** 이 아이템을 주웠을 때 인벤토리에 들어갈 내용물입니다. (설계도/인스턴스 묶음) */
	UPROPERTY(EditDefaultsOnly, Category = "변수")
	FInventoryPickup StaticInventory;

};
