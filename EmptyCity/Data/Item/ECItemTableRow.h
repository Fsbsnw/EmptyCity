#pragma once

#include "CoreMinimal.h"
#include "Data/Workbench/ECWorkbenchUpgradeTypes.h"
#include "Engine/DataTable.h"
#include "ECItemTableRow.generated.h"

class UStaticMesh;
class UTexture2D;
class UECInventoryItemDefinition;

/** 플레이어 인벤토리 화면에서 사용할 표시 카테고리입니다. */
UENUM(BlueprintType)
enum class EECInventoryItemCategory : uint8
{
	ToolWeapon UMETA(DisplayName = "Tool & Weapon"),
	SurvivalKit UMETA(DisplayName = "Survival Kit"),
	Item UMETA(DisplayName = "Item")
};

/** 모든 아이템이 공유하는 기획 데이터를 정의하는 DT_Item의 행 구조체입니다. */
USTRUCT(BlueprintType, meta = (DisplayName = "아이템 데이터"))
struct FECItemTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 이 행의 기능을 정의하는 Item Definition 클래스입니다. */
	UPROPERTY(EditAnywhere, Category = "Item", meta = (DisplayName = "아이템 Definition"))
	TSoftClassPtr<UECInventoryItemDefinition> ItemDefinition;

	/** 플레이어 인벤토리 화면에서 이 아이템을 표시할 카테고리입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (DisplayName = "인벤토리 표시 카테고리"))
	EECInventoryItemCategory InventoryCategory = EECInventoryItemCategory::Item;

	/** UI에 표시할 아이템 이름입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (DisplayName = "아이템 이름"))
	FText DisplayName;

	/** 아이템의 기본 설명입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (DisplayName = "아이템 설명", MultiLine = true))
	FText Description;

	/** 아이템이 잠겨 있을 때 표시할 설명입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (DisplayName = "잠금 상태 설명", MultiLine = true))
	FText LockedDescription;

	/** 아이템 한 개의 무게입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (DisplayName = "무게", ClampMin = "0.0", Units = "kg"))
	float Weight = 0.0f;

	/** Axe/Pickaxe/Weapon 강화에 따라 무게가 달라지는 장비를 지정합니다. MAX는 강화 대상이 아님을 뜻합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench", meta = (DisplayName = "장비 강화 타입"))
	EECWorkbenchUpgradeType ToolUpgradeType = EECWorkbenchUpgradeType::MAX;

	/** 인벤토리 한 슬롯에 들어갈 수 있는 최대 개수입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (DisplayName = "최대 스택 수", ClampMin = "1"))
	int32 StackMax = 1;

	/** 교환소에서 아이템 한 개를 구매할 때 필요한 Coin입니다. 0이면 구매할 수 없습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trade", meta = (DisplayName = "구매 가격", ClampMin = "0"))
	int32 BuyPrice = 0;

	/** 교환소에 아이템 한 개를 판매할 때 받는 Coin입니다. 0이면 판매할 수 없습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trade", meta = (DisplayName = "판매 가격", ClampMin = "0"))
	int32 SellPrice = 0;

	/** 플레이어가 게임 내내 보유해야 하는 장비입니다. 중복 지급·이동·버리기·재료 소모가 차단됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Policy", meta = (DisplayName = "영구 플레이어 아이템"))
	bool bPermanentPlayerItem = false;

	/** 교환소 구매 목록에 표시하고 구매할 수 있는지 결정합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Policy", meta = (DisplayName = "구매 가능"))
	bool bCanBuy = true;

	/** 교환소 판매 목록에 표시하고 판매할 수 있는지 결정합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Policy", meta = (DisplayName = "판매 가능"))
	bool bCanSell = true;

	/** 플레이어 인벤토리와 보관함 사이에서 이동할 수 있는지 결정합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Policy", meta = (DisplayName = "보관함 이동 가능"))
	bool bCanTransfer = true;

	/** 플레이어 인벤토리에서 버릴 수 있는지 결정합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Policy", meta = (DisplayName = "버리기 가능"))
	bool bCanDiscard = true;

	/** 인벤토리 UI에 표시할 아이콘입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (DisplayName = "아이콘"))
	TSoftObjectPtr<UTexture2D> Icon;

	/** 월드에 드롭된 아이템을 조준했을 때 공통 상호작용 인디케이터에 표시할 아이콘입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (DisplayName = "상호작용 인디케이터 아이콘"))
	TSoftObjectPtr<UTexture2D> InteractionIndicatorIcon;

	/** 아이템이 잠겨 있을 때 표시할 아이콘입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (DisplayName = "잠금 상태 아이콘"))
	TSoftObjectPtr<UTexture2D> LockedIcon;

	/** 아이템이 월드에 배치되었을 때 사용할 스태틱 메시입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World", meta = (DisplayName = "월드 스태틱 메시"))
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	/** Axe/Pickaxe 파밍 중 1인칭 캐릭터 Mesh에 부착할 소켓입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farming", meta = (DisplayName = "파밍 부착 소켓"))
	FName FarmingAttachSocket = NAME_None;

	/** 파밍 부착 소켓을 기준으로 적용할 메시 위치/회전/크기 보정값입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farming", meta = (DisplayName = "파밍 부착 보정"))
	FTransform FarmingAttachTransform = FTransform::Identity;

	/** 월드 스태틱 메시의 상대 스케일입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World", meta = (DisplayName = "월드 스케일"))
	FVector WorldScale = FVector::OneVector;
};
