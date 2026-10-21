#include "Data/Workbench/ECWorkbenchUpgradeDataAsset.h"

#include "Inventory/ECInventoryItemDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

namespace
{
	bool IsUpgradeDefinitionStructurallyValid(const FECWorkbenchUpgradeDefinition& Definition);

	bool IsUpgradeLevelStructurallyValid(const FECWorkbenchUpgradeLevelData& LevelData)
	{
		if (LevelData.Level < 1 || (LevelData.Level > 1 && LevelData.RequiredItems.IsEmpty()))
		{
			return false;
		}

		TSet<FName> ValueIds;
		for (const FECWorkbenchUpgradeValue& Value : LevelData.Values)
		{
			if (Value.ValueId.IsNone() || ValueIds.Contains(Value.ValueId))
			{
				return false;
			}
			ValueIds.Add(Value.ValueId);
		}

		TSet<UClass*> RequiredItemDefinitions;
		for (const FECInventoryItemRequirement& Requirement : LevelData.RequiredItems)
		{
			UClass* ItemDefinitionClass = Requirement.ItemDefinition.Get();
			if (!ItemDefinitionClass || Requirement.Count <= 0 || RequiredItemDefinitions.Contains(ItemDefinitionClass))
			{
				return false;
			}
			RequiredItemDefinitions.Add(ItemDefinitionClass);
		}

		return true;
	}

	bool IsUpgradeDefinitionStructurallyValid(const FECWorkbenchUpgradeDefinition& Definition)
	{
		if (Definition.UpgradeType == EECWorkbenchUpgradeType::MAX || Definition.Levels.IsEmpty())
		{
			return false;
		}

		TSet<int32> ConfiguredLevels;
		for (const FECWorkbenchUpgradeLevelData& LevelData : Definition.Levels)
		{
			if (!IsUpgradeLevelStructurallyValid(LevelData) || ConfiguredLevels.Contains(LevelData.Level))
			{
				return false;
			}
			ConfiguredLevels.Add(LevelData.Level);
		}

		for (int32 ExpectedLevel = 1; ExpectedLevel <= Definition.Levels.Num(); ++ExpectedLevel)
		{
			if (!ConfiguredLevels.Contains(ExpectedLevel))
			{
				return false;
			}
		}

		return true;
	}

	FString GetUpgradeTypeName(EECWorkbenchUpgradeType UpgradeType)
	{
		const UEnum* UpgradeTypeEnum = StaticEnum<EECWorkbenchUpgradeType>();
		return UpgradeTypeEnum
			? UpgradeTypeEnum->GetNameStringByValue(static_cast<int64>(UpgradeType))
			: TEXT("Unknown");
	}
}

const FECWorkbenchUpgradeDefinition* UECWorkbenchUpgradeDataAsset::FindUpgradeDefinition(
	EECWorkbenchUpgradeType UpgradeType) const
{
	if (UpgradeType == EECWorkbenchUpgradeType::MAX)
	{
		return nullptr;
	}

	const FECWorkbenchUpgradeDefinition* FoundDefinition = nullptr;
	for (const FECWorkbenchUpgradeDefinition& Definition : UpgradeDefinitions)
	{
		if (Definition.UpgradeType == UpgradeType)
		{
			// 에셋 전체를 채우기 전에도 완성된 타입 하나씩 미리 볼 수 있게 하되,
			// 대상 타입 자체가 잘못됐거나 중복되면 런타임 사용을 거부합니다.
			if (FoundDefinition || !IsUpgradeDefinitionStructurallyValid(Definition))
			{
				return nullptr;
			}

			FoundDefinition = &Definition;
		}
	}

	return FoundDefinition;
}

const FECWorkbenchUpgradeLevelData* UECWorkbenchUpgradeDataAsset::FindLevelData(
	EECWorkbenchUpgradeType UpgradeType,
	int32 Level) const
{
	const FECWorkbenchUpgradeDefinition* Definition = FindUpgradeDefinition(UpgradeType);
	if (!Definition)
	{
		return nullptr;
	}

	return Definition->Levels.FindByPredicate(
		[Level](const FECWorkbenchUpgradeLevelData& LevelData)
		{
			return LevelData.Level == Level;
		});
}

const FECWorkbenchUpgradeLevelData* UECWorkbenchUpgradeDataAsset::FindNextLevelData(
	EECWorkbenchUpgradeType UpgradeType,
	int32 CurrentLevel) const
{
	if (CurrentLevel < 1 || CurrentLevel == MAX_int32)
	{
		return nullptr;
	}

	return FindLevelData(UpgradeType, CurrentLevel + 1);
}

bool UECWorkbenchUpgradeDataAsset::GetUpgradeDefinition(
	EECWorkbenchUpgradeType UpgradeType,
	FECWorkbenchUpgradeDefinition& OutDefinition) const
{
	const FECWorkbenchUpgradeDefinition* Definition = FindUpgradeDefinition(UpgradeType);
	if (!Definition)
	{
		return false;
	}

	OutDefinition = *Definition;
	return true;
}

bool UECWorkbenchUpgradeDataAsset::GetLevelData(
	EECWorkbenchUpgradeType UpgradeType,
	int32 Level,
	FECWorkbenchUpgradeLevelData& OutLevelData) const
{
	const FECWorkbenchUpgradeLevelData* LevelData = FindLevelData(UpgradeType, Level);
	if (!LevelData)
	{
		return false;
	}

	OutLevelData = *LevelData;
	return true;
}

bool UECWorkbenchUpgradeDataAsset::GetNextLevelData(
	EECWorkbenchUpgradeType UpgradeType,
	int32 CurrentLevel,
	FECWorkbenchUpgradeLevelData& OutLevelData) const
{
	const FECWorkbenchUpgradeLevelData* LevelData = FindNextLevelData(UpgradeType, CurrentLevel);
	if (!LevelData)
	{
		return false;
	}

	OutLevelData = *LevelData;
	return true;
}

int32 UECWorkbenchUpgradeDataAsset::GetMaxConfiguredLevel(EECWorkbenchUpgradeType UpgradeType) const
{
	const FECWorkbenchUpgradeDefinition* Definition = FindUpgradeDefinition(UpgradeType);
	if (!Definition)
	{
		return 0;
	}

	return Definition->Levels.Num();
}

#if WITH_EDITOR
EDataValidationResult UECWorkbenchUpgradeDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;
	auto AddError = [&Context, &Result](const FString& Message)
	{
		Context.AddError(FText::FromString(Message));
		Result = EDataValidationResult::Invalid;
	};

	if (UpgradeDefinitions.IsEmpty())
	{
		AddError(TEXT("UpgradeDefinitions가 비어 있습니다."));
	}
	else if (UpgradeDefinitions.Num() != static_cast<int32>(EECWorkbenchUpgradeType::MAX))
	{
		AddError(FString::Printf(
			TEXT("UpgradeDefinitions에는 정확히 %d개의 강화 타입이 필요합니다. 현재: %d"),
			static_cast<int32>(EECWorkbenchUpgradeType::MAX),
			UpgradeDefinitions.Num()));
	}

	TSet<EECWorkbenchUpgradeType> ConfiguredTypes;
	for (int32 DefinitionIndex = 0; DefinitionIndex < UpgradeDefinitions.Num(); ++DefinitionIndex)
	{
		const FECWorkbenchUpgradeDefinition& Definition = UpgradeDefinitions[DefinitionIndex];
		const FString TypeName = GetUpgradeTypeName(Definition.UpgradeType);

		if (Definition.UpgradeType == EECWorkbenchUpgradeType::MAX)
		{
			AddError(FString::Printf(TEXT("UpgradeDefinitions[%d]에 MAX 타입을 사용할 수 없습니다."), DefinitionIndex));
		}

		if (ConfiguredTypes.Contains(Definition.UpgradeType))
		{
			AddError(FString::Printf(TEXT("UpgradeType이 중복되었습니다: %s"), *TypeName));
		}
		else
		{
			ConfiguredTypes.Add(Definition.UpgradeType);
		}

		if (Definition.Levels.IsEmpty())
		{
			AddError(FString::Printf(TEXT("%s의 Levels가 비어 있습니다."), *TypeName));
			continue;
		}

		TSet<int32> ConfiguredLevels;
		for (int32 LevelIndex = 0; LevelIndex < Definition.Levels.Num(); ++LevelIndex)
		{
			const FECWorkbenchUpgradeLevelData& LevelData = Definition.Levels[LevelIndex];
			if (LevelData.Level < 1)
			{
				AddError(FString::Printf(TEXT("%s Levels[%d]의 Level은 1 이상이어야 합니다."), *TypeName, LevelIndex));
			}
			else if (ConfiguredLevels.Contains(LevelData.Level))
			{
				AddError(FString::Printf(TEXT("%s에 Level %d가 중복되었습니다."), *TypeName, LevelData.Level));
			}
			else
			{
				ConfiguredLevels.Add(LevelData.Level);
			}

			TSet<FName> ValueIds;
			for (int32 ValueIndex = 0; ValueIndex < LevelData.Values.Num(); ++ValueIndex)
			{
				const FName ValueId = LevelData.Values[ValueIndex].ValueId;
				if (ValueId.IsNone())
				{
					AddError(FString::Printf(TEXT("%s Level %d Values[%d]의 ValueId가 None입니다."), *TypeName, LevelData.Level, ValueIndex));
				}
				else if (ValueIds.Contains(ValueId))
				{
					AddError(FString::Printf(TEXT("%s Level %d의 ValueId가 중복되었습니다: %s"), *TypeName, LevelData.Level, *ValueId.ToString()));
				}
				else
				{
					ValueIds.Add(ValueId);
				}
			}

			TSet<UClass*> RequiredItemDefinitions;
			if (LevelData.Level > 1 && LevelData.RequiredItems.IsEmpty())
			{
				AddError(FString::Printf(TEXT("%s Level %d에는 최소 1개의 강화 비용이 필요합니다."), *TypeName, LevelData.Level));
			}

			for (int32 RequirementIndex = 0; RequirementIndex < LevelData.RequiredItems.Num(); ++RequirementIndex)
			{
				const FECInventoryItemRequirement& Requirement = LevelData.RequiredItems[RequirementIndex];
				UClass* ItemDefinitionClass = Requirement.ItemDefinition.Get();
				if (!ItemDefinitionClass)
				{
					AddError(FString::Printf(TEXT("%s Level %d RequiredItems[%d]의 ItemDefinition이 비어 있습니다."), *TypeName, LevelData.Level, RequirementIndex));
				}
				if (Requirement.Count <= 0)
				{
					AddError(FString::Printf(TEXT("%s Level %d RequiredItems[%d]의 Count는 1 이상이어야 합니다."), *TypeName, LevelData.Level, RequirementIndex));
				}
				if (ItemDefinitionClass && RequiredItemDefinitions.Contains(ItemDefinitionClass))
				{
					AddError(FString::Printf(TEXT("%s Level %d의 ItemDefinition 비용이 중복되었습니다: %s"), *TypeName, LevelData.Level, *GetNameSafe(ItemDefinitionClass)));
				}
				else if (ItemDefinitionClass)
				{
					RequiredItemDefinitions.Add(ItemDefinitionClass);
				}
			}
		}

		for (int32 ExpectedLevel = 1; ExpectedLevel <= Definition.Levels.Num(); ++ExpectedLevel)
		{
			if (!ConfiguredLevels.Contains(ExpectedLevel))
			{
				AddError(FString::Printf(TEXT("%s의 Levels는 1부터 연속이어야 합니다. 누락 단계: %d"), *TypeName, ExpectedLevel));
			}
		}
	}

	for (int32 TypeValue = 0; TypeValue < static_cast<int32>(EECWorkbenchUpgradeType::MAX); ++TypeValue)
	{
		const EECWorkbenchUpgradeType RequiredType = static_cast<EECWorkbenchUpgradeType>(TypeValue);
		if (!ConfiguredTypes.Contains(RequiredType))
		{
			AddError(FString::Printf(TEXT("필수 UpgradeType이 누락되었습니다: %s"), *GetUpgradeTypeName(RequiredType)));
		}
	}

	return CombineDataValidationResults(Result, Super::IsDataValid(Context));
}
#endif
