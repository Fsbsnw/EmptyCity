#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ECFarmingTableRow.generated.h"

class UAnimMontage;
class UECEquipmentDefinition;

/** 파밍 대상의 상호작용과 연출 설정을 정의하는 행 구조체입니다. */
USTRUCT()
struct FECFarmingTableRow : public FTableRowBase
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Farming
// ─────────────────────────────────────────────────────────────
public:
	/** 상호작용 UI에 표시할 이름입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|표시")
	FText InteractionText;

	/** 파밍을 완료하기 위해 입력을 유지할 시간입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|상호작용", meta = (ClampMin = "0.1", Units = "s"))
	float InteractionDuration = 1.0f;

	/** 파밍 중 임시로 장착할 도구 장비입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|도구")
	TSubclassOf<UECEquipmentDefinition> ToolEquipmentDefinition;

	/** 파밍 중 재생할 몽타주입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|애니메이션")
	TSoftObjectPtr<UAnimMontage> FarmingMontage;
};
