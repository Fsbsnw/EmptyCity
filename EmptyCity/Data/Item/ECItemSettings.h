#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ECItemSettings.generated.h"

class UDataTable;

/** 아이템 시스템이 공통으로 사용하는 데이터 에셋 경로를 관리합니다. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Item Settings"))
class EMPTYCITY_API UECItemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * 모든 아이템의 이름, 설명, 무게, 최대 스택 수와 표현 데이터를 담는 테이블입니다.
	 * 각 행은 Item Definition을 직접 참조하며 RowName을 해당 아이템의 논리 식별자로 사용합니다.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Item Data", meta = (DisplayName = "Item Data Table", RequiredAssetDataTags = "RowStructure=/Script/EmptyCity.ECItemTableRow"))
	TSoftObjectPtr<UDataTable> ItemDataTable;
};
