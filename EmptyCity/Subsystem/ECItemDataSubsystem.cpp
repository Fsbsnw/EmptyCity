#include "Subsystem/ECItemDataSubsystem.h"
#include "Data/Item/ECItemSettings.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "Inventory/ECInventoryItemInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogECItemData, Log, All);

void UECItemDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	DefinitionToItemId.Reset();

	const UECItemSettings* Settings = GetDefault<UECItemSettings>();
	if (!Settings || Settings->ItemDataTable.IsNull())
	{
		UE_LOG(LogECItemData, Error, TEXT("Item Settings에 Item Data Table이 설정되지 않았습니다."));
		return;
	}

	ItemDataTable = Settings->ItemDataTable.LoadSynchronous();
	if (!IsValid(ItemDataTable))
	{
		UE_LOG(LogECItemData, Error, TEXT("Item Data Table을 로드하지 못했습니다: %s"), *Settings->ItemDataTable.ToSoftObjectPath().ToString());
		return;
	}

	if (ItemDataTable->GetRowStruct() != FECItemTableRow::StaticStruct())
	{
		UE_LOG(LogECItemData, Error, TEXT("Item Data Table의 행 구조가 FECItemTableRow가 아닙니다: %s"), *GetNameSafe(ItemDataTable));
		ItemDataTable = nullptr;
		return;
	}

	for (const TPair<FName, uint8*>& RowPair : ItemDataTable->GetRowMap())
	{
		const FECItemTableRow* ItemData = reinterpret_cast<const FECItemTableRow*>(RowPair.Value);
		if (!ItemData || ItemData->ItemDefinition.IsNull())
		{
			UE_LOG(LogECItemData, Warning, TEXT("Item Definition이 지정되지 않은 행입니다: %s"), *RowPair.Key.ToString());
			continue;
		}

		const FSoftObjectPath DefinitionPath = ItemData->ItemDefinition.ToSoftObjectPath();
		if (const FName* ExistingItemId = DefinitionToItemId.Find(DefinitionPath))
		{
			UE_LOG(LogECItemData, Error, TEXT("동일한 Item Definition을 참조하는 행이 중복되었습니다: %s, %s (%s)"), *ExistingItemId->ToString(), *RowPair.Key.ToString(), *DefinitionPath.ToString());
			continue;
		}

		DefinitionToItemId.Add(DefinitionPath, RowPair.Key);
	}

	UE_LOG(LogECItemData, Log, TEXT("Item Data Table 로드 완료: %s (%d개 행, %d개 Definition)"), *GetNameSafe(ItemDataTable), ItemDataTable->GetRowMap().Num(), DefinitionToItemId.Num());
}

void UECItemDataSubsystem::Deinitialize()
{
	DefinitionToItemId.Reset();
	ItemDataTable = nullptr;
	Super::Deinitialize();
}

UECItemDataSubsystem& UECItemDataSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	check(World);
	UECItemDataSubsystem* Subsystem = UGameInstance::GetSubsystem<UECItemDataSubsystem>(World->GetGameInstance());
	check(Subsystem);
	return *Subsystem;
}

const FECItemTableRow* UECItemDataSubsystem::FindItemData(FName ItemId) const
{
	if (ItemId.IsNone() || !IsValid(ItemDataTable))
	{
		return nullptr;
	}

	return ItemDataTable->FindRow<FECItemTableRow>(ItemId, TEXT("UECItemDataSubsystem::FindItemData"), false);
}

FName UECItemDataSubsystem::FindItemId(TSubclassOf<UECInventoryItemDefinition> ItemDefinition) const
{
	if (!ItemDefinition)
	{
		return NAME_None;
	}

	const FName* ItemId = DefinitionToItemId.Find(FSoftObjectPath(ItemDefinition.Get()));
	return *ItemId;
}

FName UECItemDataSubsystem::FindItemId(const UECInventoryItemInstance* ItemInstance) const
{
	return FindItemId(ItemInstance->ItemDef);
}

const FECItemTableRow* UECItemDataSubsystem::FindItemData(TSubclassOf<UECInventoryItemDefinition> ItemDefinition) const
{
	return FindItemData(FindItemId(ItemDefinition));
}

const FECItemTableRow* UECItemDataSubsystem::FindItemData(const UECInventoryItemInstance* ItemInstance) const
{
	return FindItemData(FindItemId(ItemInstance));
}
