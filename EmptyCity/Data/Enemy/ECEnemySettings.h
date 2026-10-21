#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ECEnemySettings.generated.h"

class UDataTable;

/** 적 데이터 시스템이 공통으로 사용하는 데이터 에셋 경로를 관리합니다. */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Enemy Settings"))
class EMPTYCITY_API UECEnemySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Enemy Character Class와 식별 태그, 표시 이름의 매핑을 담는 테이블입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Enemy Data",
		meta=(DisplayName="Enemy Data Table", RequiredAssetDataTags="RowStructure=/Script/EmptyCity.ECEnemyTableRow"))
	TSoftObjectPtr<UDataTable> EnemyDataTable;
};
