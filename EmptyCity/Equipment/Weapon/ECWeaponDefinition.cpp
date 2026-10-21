#include "ECWeaponDefinition.h"
#include "ECWeaponInstance.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

UECWeaponDefinition::UECWeaponDefinition()
{
	// WeaponDefinition은 기본적으로 WeaponInstance를 생성합니다.
	InstanceType = UECWeaponInstance::StaticClass();

	// 현재 플레이어 무기의 기본 강화 단계별 외형입니다.
	// 파생 WeaponDefinition에서는 UpgradeLevelVisualMeshes를 재정의할 수 있습니다.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> LevelOneMesh(
		TEXT("/Game/_EmptyCity/Maps/Art/StaticMesh/Weapon/PlayerWeapon/SM_WoodenWeapon_A01.SM_WoodenWeapon_A01"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> LevelTwoMesh(
		TEXT("/Game/_EmptyCity/Maps/Art/StaticMesh/Weapon/PlayerWeapon/SM_PipeWeapon_A01.SM_PipeWeapon_A01"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> LevelThreeMesh(
		TEXT("/Game/_EmptyCity/Maps/Art/StaticMesh/Weapon/PlayerWeapon/SM_SteelWeapon_A01.SM_SteelWeapon_A01"));

	UpgradeLevelVisualMeshes.SetNum(3);
	UpgradeLevelVisualMeshes[0] = LevelOneMesh.Object;
	UpgradeLevelVisualMeshes[1] = LevelTwoMesh.Object;
	UpgradeLevelVisualMeshes[2] = LevelThreeMesh.Object;
}

const FECWeaponTableRow* UECWeaponDefinition::GetWeaponData() const
{
	return WeaponDataRow.GetRow<FECWeaponTableRow>(TEXT("UECWeaponDefinition::GetWeaponData"));
}

UStaticMesh* UECWeaponDefinition::GetVisualMeshForUpgradeLevel(int32 UpgradeLevel) const
{
	if (UpgradeLevelVisualMeshes.IsEmpty())
	{
		return nullptr;
	}

	const int32 MeshIndex = FMath::Clamp(UpgradeLevel - 1, 0, UpgradeLevelVisualMeshes.Num() - 1);
	return UpgradeLevelVisualMeshes[MeshIndex];
}
