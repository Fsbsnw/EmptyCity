#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystem/ECTimeTypes.h"
#include "Subsystem/ECTutorialTypes.h"
#include "ECSaveTypes.generated.h"

class UECInventoryItemDefinition;

USTRUCT(BlueprintType)
struct FECPlayerSaveData
{
	GENERATED_BODY()

	/** 플레이어가 보유한 재화입니다. 인벤토리 아이템과 무관한 숫자 데이터로 저장됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Player")
	int32 Coin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Player")
	int32 HealthLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Player")
	int32 StaminaLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Player")
	int32 MoveSpeedLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Player")
	int32 CombatLevel = 1;

	/** 작업대에서 강화한 최대 인벤토리 무게 단계입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Player")
	int32 MaxInventoryWeightLevel = 1;

	/** 작업대에서 강화한 도끼 단계입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Player")
	int32 AxeLevel = 1;

	/** 작업대에서 강화한 곡괭이 단계입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Player")
	int32 PickaxeLevel = 1;

	/** 작업대에서 강화한 무기 단계입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Player")
	int32 WeaponLevel = 1;
};

USTRUCT(BlueprintType)
struct FECTimeSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Time")
	int32 Day = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Time")
	EECTimeOfDay TimeOfDay = EECTimeOfDay::Morning;
};

/** 성공한 지역 이동 한 건을 나타내는 방문 기록입니다. */
USTRUCT(BlueprintType)
struct FECRegionVisitRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Region")
	FGameplayTag RegionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Region")
	int32 Day = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Region")
	EECTimeOfDay TimeOfDay = EECTimeOfDay::Morning;
};

USTRUCT(BlueprintType)
struct FECRegionSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Region")
	TArray<FECRegionVisitRecord> VisitHistory;
};

USTRUCT(BlueprintType)
struct FECInventoryItemSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Inventory")
	FGuid InstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Inventory")
	TSoftClassPtr<UECInventoryItemDefinition> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Inventory")
	int32 StackCount = 1;
};

USTRUCT(BlueprintType)
struct FECInventorySaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save|Inventory")
	TArray<FECInventoryItemSaveData> Items;
};

USTRUCT(BlueprintType)
struct FECTutorialSaveData
{
	GENERATED_BODY()

	/** 구버전 세이브와 튜토리얼 상태가 기록된 세이브를 구분합니다. */
	UPROPERTY(SaveGame)
	bool bInitialized = false;

	UPROPERTY(SaveGame)
	EECTutorialStage Stage = EECTutorialStage::Movement;

	UPROPERTY(SaveGame)
	bool bSprintUsed = false;

	UPROPERTY(SaveGame)
	bool bAttackUsed = false;

	UPROPERTY(SaveGame)
	bool bGuardUsed = false;
};

USTRUCT(BlueprintType)
struct FECGameplaySaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save")
	FECTimeSaveData Time;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save")
	FECRegionSaveData Region;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save")
	FECPlayerSaveData Player;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save")
	FECInventorySaveData Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save")
	FECTutorialSaveData Tutorial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save")
	FGameplayTagContainer CompletedBossTags;

	/** 한 번만 재생하는 컷씬 중 재생을 완료한 컷씬 태그입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save")
	FGameplayTagContainer CompletedCutsceneTags;

	/** 로드 시 플레이어를 배치할 지역 텔레포트 지점 태그입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save")
	FGameplayTag SpawnPointTag;
};
