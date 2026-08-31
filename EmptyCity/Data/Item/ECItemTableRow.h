#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ECItemTableRow.generated.h"

class UStaticMesh;
class UTexture2D;
class UECInventoryItemDefinition;

/** 모든 아이템이 공유하는 기획 데이터를 정의하는 DT_Item의 행 구조체입니다. */
USTRUCT(BlueprintType, meta = (DisplayName = "아이템 데이터"))
struct FECItemTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 이 행의 기능을 정의하는 Item Definition 클래스입니다. */
	UPROPERTY(EditAnywhere, Category = "Item", meta = (DisplayName = "아이템 Definition"))
	TSoftClassPtr<UECInventoryItemDefinition> ItemDefinition;

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

	/** 인벤토리 한 슬롯에 들어갈 수 있는 최대 개수입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (DisplayName = "최대 스택 수", ClampMin = "1"))
	int32 StackMax = 1;

	/** 인벤토리 UI에 표시할 아이콘입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (DisplayName = "아이콘"))
	TSoftObjectPtr<UTexture2D> Icon;

	/** 아이템이 잠겨 있을 때 표시할 아이콘입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (DisplayName = "잠금 상태 아이콘"))
	TSoftObjectPtr<UTexture2D> LockedIcon;

	/** 아이템이 월드에 배치되었을 때 사용할 스태틱 메시입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World", meta = (DisplayName = "월드 스태틱 메시"))
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	/** 월드 스태틱 메시의 상대 스케일입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World", meta = (DisplayName = "월드 스케일"))
	FVector WorldScale = FVector::OneVector;
};
