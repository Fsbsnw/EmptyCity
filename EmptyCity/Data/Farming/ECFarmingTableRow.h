#pragma once

#include "CoreMinimal.h"
#include "Data/Workbench/ECWorkbenchUpgradeTypes.h"
#include "Engine/DataTable.h"
#include "ECFarmingTableRow.generated.h"

class UAnimMontage;
class UTexture2D;

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

	/** 공통 상호작용 인디케이터에 표시할 파밍 대상 아이콘입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|표시")
	TSoftObjectPtr<UTexture2D> InteractionIcon;

	/** 파밍을 완료하기 위해 입력을 유지할 시간입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|상호작용", meta = (ClampMin = "0.1", Units = "s"))
	float InteractionDuration = 1.0f;

	/** 이 파밍 대상에 적용할 작업대 도구 강화 타입입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|도구")
	EECWorkbenchUpgradeType ToolUpgradeType = EECWorkbenchUpgradeType::MAX;

	/** 이 파밍 대상에서 1인칭 Mesh로 재생할 몽타주입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|애니메이션")
	TSoftObjectPtr<UAnimMontage> FarmingMontage;

	/** 파밍 애니메이션의 타격 Notify에서 재생할 DT_Sound 행입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|사운드", meta = (RowType = "/Script/EmptyCity.SoundTableRow"))
	FDataTableRowHandle FarmingSound;
};
