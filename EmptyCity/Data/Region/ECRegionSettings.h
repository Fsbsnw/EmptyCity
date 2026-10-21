#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ECRegionSettings.generated.h"

class UDataTable;

/** 지역 시스템이 공통으로 사용하는 데이터 에셋 경로를 관리합니다. */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Region Settings"))
class EMPTYCITY_API UECRegionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * 지역별 표시 데이터를 담는 테이블입니다.
	 * RowName은 MapNode.Location.Shelter처럼 지역 GameplayTag 이름과 동일해야 합니다.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Region Data",
		meta=(DisplayName="Region Definition Table", RequiredAssetDataTags="RowStructure=/Script/EmptyCity.ECRegionTableRow"))
	TSoftObjectPtr<UDataTable> RegionDefinitionTable;
};
