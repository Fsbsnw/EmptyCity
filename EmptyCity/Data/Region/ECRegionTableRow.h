#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "ECRegionTableRow.generated.h"

class UTexture2D;

/** GameplayTag로 식별되는 지역의 공통 표시 데이터입니다. */
USTRUCT(BlueprintType)
struct FECRegionTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 선택한 지역의 정보를 보여주는 이미지입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Region")
	TObjectPtr<UTexture2D> RegionInfoImage = nullptr;

	/** 선택한 지역의 실제 전경 또는 대표 이미지를 보여줍니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Region")
	TObjectPtr<UTexture2D> RegionImage = nullptr;
	
	/** 선택한 지역의 방문 정보를 보여주는 이미지입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Region")
	TObjectPtr<UTexture2D> RegionVisitHistoryImage = nullptr;

	/** 이 지역을 처음 방문할 때 재생할 컷씬입니다. 비어 있으면 최초 방문 연출을 생략합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Region", meta=(Categories="Cutscene"))
	FGameplayTag FirstVisitCutsceneTag;
};
