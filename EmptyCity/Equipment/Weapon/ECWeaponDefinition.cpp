#include "ECWeaponDefinition.h"
#include "ECWeaponInstance.h"

UECWeaponDefinition::UECWeaponDefinition()
{
	// WeaponDefinition은 기본적으로 WeaponInstance를 생성합니다.
	InstanceType = UECWeaponInstance::StaticClass();
}

const FECWeaponTableRow* UECWeaponDefinition::GetWeaponData() const
{
	return WeaponDataRow.GetRow<FECWeaponTableRow>(TEXT("UECWeaponDefinition::GetWeaponData"));
}
