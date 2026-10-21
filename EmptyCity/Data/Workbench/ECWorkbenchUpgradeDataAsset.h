#pragma once

#include "CoreMinimal.h"
#include "Data/Workbench/ECWorkbenchUpgradeTypes.h"
#include "Engine/DataAsset.h"
#include "Inventory/ECInventoryItemRequirement.h"
#include "ECWorkbenchUpgradeDataAsset.generated.h"

/**
 * 단계별 표시/적용 값을 이름과 숫자로 저장하는 확장 가능한 값입니다.
 * 예: MaxHealth, Weight, InteractionDuration, AttackPower, AttackSpeedMultiplier.
 */
USTRUCT(BlueprintType)
struct EMPTYCITY_API FECWorkbenchUpgradeValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Upgrade")
	FName ValueId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Upgrade")
	float Value = 0.0f;
};

/**
 * 특정 강화 단계의 데이터입니다.
 * RequiredItems는 직전 단계에서 이 Level로 올라갈 때 소모되는 재료입니다.
 * 1단계는 기본 단계이므로 일반적으로 RequiredItems를 비워 둡니다.
 */
USTRUCT(BlueprintType)
struct EMPTYCITY_API FECWorkbenchUpgradeLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Upgrade", meta = (ClampMin = "1", UIMin = "1"))
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Upgrade")
	TArray<FECWorkbenchUpgradeValue> Values;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Upgrade")
	TArray<FECInventoryItemRequirement> RequiredItems;
};

/** 강화 대상 하나와 그 대상이 가질 수 있는 모든 단계를 정의합니다. */
USTRUCT(BlueprintType)
struct EMPTYCITY_API FECWorkbenchUpgradeDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Upgrade")
	EECWorkbenchUpgradeType UpgradeType = EECWorkbenchUpgradeType::MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Upgrade")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Upgrade")
	TArray<FECWorkbenchUpgradeLevelData> Levels;
};

/**
 * 스탯/도구/무기의 단계 값과 강화 비용을 한 곳에서 조회하는 데이터 에셋입니다.
 * 여섯 강화 타입을 모두 정의하며, 최대 단계는 유효한 연속 Levels 개수에서 계산합니다.
 */
UCLASS(BlueprintType)
class EMPTYCITY_API UECWorkbenchUpgradeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	const FECWorkbenchUpgradeDefinition* FindUpgradeDefinition(EECWorkbenchUpgradeType UpgradeType) const;
	const FECWorkbenchUpgradeLevelData* FindLevelData(EECWorkbenchUpgradeType UpgradeType, int32 Level) const;
	const FECWorkbenchUpgradeLevelData* FindNextLevelData(EECWorkbenchUpgradeType UpgradeType, int32 CurrentLevel) const;

	UFUNCTION(BlueprintPure, Category = "Workbench|Upgrade")
	bool GetUpgradeDefinition(EECWorkbenchUpgradeType UpgradeType, FECWorkbenchUpgradeDefinition& OutDefinition) const;

	UFUNCTION(BlueprintPure, Category = "Workbench|Upgrade")
	bool GetLevelData(EECWorkbenchUpgradeType UpgradeType, int32 Level, FECWorkbenchUpgradeLevelData& OutLevelData) const;

	UFUNCTION(BlueprintPure, Category = "Workbench|Upgrade")
	bool GetNextLevelData(EECWorkbenchUpgradeType UpgradeType, int32 CurrentLevel, FECWorkbenchUpgradeLevelData& OutLevelData) const;

	UFUNCTION(BlueprintPure, Category = "Workbench|Upgrade")
	int32 GetMaxConfiguredLevel(EECWorkbenchUpgradeType UpgradeType) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Upgrade")
	TArray<FECWorkbenchUpgradeDefinition> UpgradeDefinitions;
};
