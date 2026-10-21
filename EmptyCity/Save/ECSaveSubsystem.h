#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Save/ECSaveTypes.h"
#include "ECSaveSubsystem.generated.h"

UCLASS()
class EMPTYCITY_API UECSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	bool SaveCurrentGame();
	bool LoadCurrentGame();
	bool HasSaveGame() const;
	bool IsLoadInProgress() const { return bLoadInProgress; }

	void RegisterSaveable(UObject* Saveable);
	void UnregisterSaveable(UObject* Saveable);

private:
	bool BuildSaveData(FECGameplaySaveData& OutSaveData) const;
	bool ApplySaveData(const FECGameplaySaveData& SaveData);

	bool WriteSaveGame(const FECGameplaySaveData& SaveData) const;
	bool ReadSaveGame(FECGameplaySaveData& OutSaveData) const;

private:
	UPROPERTY()
	TArray<TObjectPtr<UObject>> Saveables;

	/** 로드 도중 발생한 시간/지역 이벤트가 체크포인트를 다시 저장하지 않도록 구분합니다. */
	bool bLoadInProgress = false;

	static const FString SlotName;
	static constexpr int32 UserIndex = 0;
};
