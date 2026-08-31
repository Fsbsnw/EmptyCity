#pragma once

#include "CoreMinimal.h"
#include "Data/Weapon/ECWeaponTableRow.h"
#include "Equipment/ECEquipmentDefinition.h"
#include "ECWeaponDefinition.generated.h"

UCLASS()
class EMPTYCITY_API UECWeaponDefinition : public UECEquipmentDefinition
{
	GENERATED_BODY()
	
public:
	UECWeaponDefinition();

	/** 연결된 무기 데이터 테이블 행을 반환합니다. */
	const FECWeaponTableRow* GetWeaponData() const;

protected:
	/** 이 무기가 사용할 DT_Weapon의 행입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수")
	FDataTableRowHandle WeaponDataRow;
};
