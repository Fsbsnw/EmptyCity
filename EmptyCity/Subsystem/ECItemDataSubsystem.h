#pragma once

#include "CoreMinimal.h"
#include "Data/Item/ECItemTableRow.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ECItemDataSubsystem.generated.h"

class UDataTable;
class UECInventoryItemDefinition;
class UECInventoryItemInstance;

/**
 * DT_Item을 로드하고 ItemId를 기준으로 공통 아이템 기획 데이터를 제공합니다.
 */
UCLASS()
class EMPTYCITY_API UECItemDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// GameInstanceSubsystem Interface
// ─────────────────────────────────────────────────────────────
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;


// ─────────────────────────────────────────────────────────────
// Singleton
// ─────────────────────────────────────────────────────────────
public:
	/** WorldContextObject으로 ECItemDataSubsystem을 반환합니다. */
	static UECItemDataSubsystem& Get(const UObject* WorldContextObject);


// ─────────────────────────────────────────────────────────────
// Item Data Query
// ─────────────────────────────────────────────────────────────
public:
	/** ItemId와 동일한 이름의 DT_Item 행을 반환합니다. 찾지 못하면 nullptr을 반환합니다. */
	const FECItemTableRow* FindItemData(FName ItemId) const;

	/** DT_Item에서 Item Definition을 참조하는 행의 ItemId를 반환합니다. */
	FName FindItemId(TSubclassOf<UECInventoryItemDefinition> ItemDefinition) const;

	/** DT_Item에서 Item Instance의 Definition을 참조하는 행의 ItemId를 반환합니다. */
	FName FindItemId(const UECInventoryItemInstance* ItemInstance) const;

	/** DT_Item에서 Item Definition을 참조하는 행을 반환합니다. */
	const FECItemTableRow* FindItemData(TSubclassOf<UECInventoryItemDefinition> ItemDefinition) const;

	/** DT_Item에서 Item Instance의 Definition을 참조하는 행을 반환합니다. */
	const FECItemTableRow* FindItemData(const UECInventoryItemInstance* ItemInstance) const;


// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
private:
	/** 프로젝트 설정을 통해 로드한 DT_Item입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> ItemDataTable;

	/** Item Definition의 Soft Class 경로로 DT_Item RowName을 찾는 역방향 캐시입니다. */
	TMap<FSoftObjectPath, FName> DefinitionToItemId;
};
