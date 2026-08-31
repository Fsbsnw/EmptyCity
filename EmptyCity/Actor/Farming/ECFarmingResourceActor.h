#pragma once

#include "CoreMinimal.h"
#include "Data/Farming/ECFarmingTableRow.h"
#include "GameFramework/Actor.h"
#include "Interaction/IInteractableTarget.h"
#include "ECFarmingResourceActor.generated.h"

class UECItemDropComponent;
class UStaticMeshComponent;

/** DT_Farming의 한 행으로 상호작용과 아이템 드롭을 구성하는 파밍 대상입니다. */
UCLASS()
class EMPTYCITY_API AECFarmingResourceActor : public AActor, public IInteractableTarget
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Actor Interface
// ─────────────────────────────────────────────────────────────
public:
	/** 파밍 대상의 메시, 콜리전 및 드롭 컴포넌트를 생성합니다. */
	AECFarmingResourceActor();


// ─────────────────────────────────────────────────────────────
// InteractableTarget Interface
// ─────────────────────────────────────────────────────────────
public:
	/** 파밍 데이터로 플레이어에게 제공할 상호작용 옵션을 추가합니다. */
	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& OptionBuilder) override;


// ─────────────────────────────────────────────────────────────
// Farming
// ─────────────────────────────────────────────────────────────
public:
	/** 이 파밍 대상에 설정된 DT_Farming 행을 반환합니다. */
	const FECFarmingTableRow& GetFarmingData() const;

	/** 아이템을 드롭하고 파밍 대상을 영구 제거합니다. */
	void CompleteFarming();


// ─────────────────────────────────────────────────────────────
// Components
// ─────────────────────────────────────────────────────────────
protected:
	UPROPERTY(VisibleAnywhere, Category = "변수|컴포넌트")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "변수|컴포넌트")
	TObjectPtr<UECItemDropComponent> ItemDropComponent;


// ─────────────────────────────────────────────────────────────
// Farming Data
// ─────────────────────────────────────────────────────────────
protected:
	UPROPERTY(EditDefaultsOnly, Category = "변수|파밍", meta = (RowType = "/Script/EmptyCity.ECFarmingTableRow"))
	FDataTableRowHandle FarmingRow;
};
