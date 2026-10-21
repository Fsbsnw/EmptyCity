#pragma once

#include "CoreMinimal.h"
#include "ECSaveTypes.h"
#include "UObject/Interface.h"
#include "ECSaveable.generated.h"


UINTERFACE()
class UECSaveable : public UInterface
{
	GENERATED_BODY()
};

/**
 * SaveSubsystem에 저장/복원 가능한 객체가 구현하는 인터페이스.
 */
class EMPTYCITY_API IECSaveable
{
	GENERATED_BODY()

public:
	/** Runtime -> SaveData */
	virtual void WriteSaveData(FECGameplaySaveData& InOutSaveData) const = 0;

	/** SaveData -> Runtime */
	virtual void ReadSaveData(const FECGameplaySaveData& SaveData) = 0;
};
