#pragma once

#include "CoreMinimal.h"
#include "Data/Weapon/ECWeaponTableRow.h"
#include "Equipment/ECEquipmentDefinition.h"
#include "ECWeaponDefinition.generated.h"

class UStaticMesh;

UCLASS()
class EMPTYCITY_API UECWeaponDefinition : public UECEquipmentDefinition
{
	GENERATED_BODY()
	
public:
	UECWeaponDefinition();

	/** 연결된 무기 데이터 테이블 행을 반환합니다. */
	const FECWeaponTableRow* GetWeaponData() const;

	/** 작업대 무기 강화 단계에 대응하는 월드 표시용 Static Mesh를 반환합니다. */
	UStaticMesh* GetVisualMeshForUpgradeLevel(int32 UpgradeLevel) const;

protected:
	/** 이 무기가 사용할 DT_Weapon의 행입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수")
	FDataTableRowHandle WeaponDataRow;

	/** 인덱스 0부터 강화 1단계에 대응하는 월드 표시용 Static Mesh 목록입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|외형")
	TArray<TObjectPtr<UStaticMesh>> UpgradeLevelVisualMeshes;
};
