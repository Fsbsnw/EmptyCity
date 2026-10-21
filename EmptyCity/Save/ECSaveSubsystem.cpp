#include "Save/ECSaveSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Save/ECSaveable.h"
#include "Save/ECSaveGame.h"
#include "Subsystem/ECRegionSubsystem.h"

const FString UECSaveSubsystem::SlotName = TEXT("MainSave");

bool UECSaveSubsystem::SaveCurrentGame()
{
	FECGameplaySaveData SaveData;

	if (!BuildSaveData(SaveData))
	{
		return false;
	}

	return WriteSaveGame(SaveData);
}

bool UECSaveSubsystem::LoadCurrentGame()
{
	FECGameplaySaveData SaveData;

	if (!ReadSaveGame(SaveData))
	{
		UE_LOG(LogTemp, Warning, TEXT("ReadSaveGame EXIT"));
		return false;
	}
	UE_LOG(LogTemp, Warning, TEXT("LoadCurrentGame ENTER"));

	bLoadInProgress = true;
	const bool bAppliedSaveData = ApplySaveData(SaveData);

	bool bRestoredSpawnPoint = false;
	if (bAppliedSaveData)
	{
		if (UECRegionSubsystem* RegionSubsystem = GetGameInstance()->GetSubsystem<UECRegionSubsystem>())
		{
			bRestoredSpawnPoint = RegionSubsystem->RestoreSavedSpawnPoint(SaveData.SpawnPointTag);
		}
	}

	bLoadInProgress = false;
	return bAppliedSaveData && bRestoredSpawnPoint;
}

bool UECSaveSubsystem::HasSaveGame() const
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

void UECSaveSubsystem::RegisterSaveable(UObject* Saveable)
{
	if (!IsValid(Saveable))
	{
		return;
	}

	if (!Saveable->GetClass()->ImplementsInterface(UECSaveable::StaticClass()))
	{
		return;
	}

	Saveables.AddUnique(Saveable);
}

void UECSaveSubsystem::UnregisterSaveable(UObject* Saveable)
{
	Saveables.Remove(Saveable);
}

bool UECSaveSubsystem::BuildSaveData(FECGameplaySaveData& OutSaveData) const
{
	for (UObject* Object : Saveables)
	{
		if (!IsValid(Object))
		{
			continue;
		}

		IECSaveable* Saveable = Cast<IECSaveable>(Object);

		if (!Saveable)
		{
			continue;
		}

		Saveable->WriteSaveData(OutSaveData);
	}

	return true;
}

bool UECSaveSubsystem::ApplySaveData(const FECGameplaySaveData& SaveData)
{
	UE_LOG(LogTemp, Warning, TEXT("ApplySaveData ENTER"));
	for (UObject* Object : Saveables)
	{
		if (!IsValid(Object))
		{
			continue;
		}

		IECSaveable* Saveable = Cast<IECSaveable>(Object);

		if (!Saveable)
		{
			continue;
		}

		Saveable->ReadSaveData(SaveData);
	}

	return true;
}

bool UECSaveSubsystem::WriteSaveGame(const FECGameplaySaveData& SaveData) const
{
	UECSaveGame* SaveGameObject = 
		Cast<UECSaveGame>(UGameplayStatics::CreateSaveGameObject(UECSaveGame::StaticClass()));

	if (!SaveGameObject)
	{
		return false;
	}

	SaveGameObject->SaveData = SaveData;

	return UGameplayStatics::SaveGameToSlot(SaveGameObject,	SlotName, UserIndex);
}

bool UECSaveSubsystem::ReadSaveGame(FECGameplaySaveData& OutSaveData) const
{
	UE_LOG(LogTemp, Warning, TEXT("ReadSaveGame ENTER"));
	if (!HasSaveGame())
	{
		return false;
	}

	UECSaveGame* SaveGameObject =
		Cast<UECSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));

	if (!SaveGameObject)
	{
		return false;
	}

	OutSaveData = SaveGameObject->SaveData;
	return true;
}
