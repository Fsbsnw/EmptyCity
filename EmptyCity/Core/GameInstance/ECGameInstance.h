#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ECGameInstance.generated.h"

class UECSoundSubsystem;

UCLASS()
class EMPTYCITY_API UECGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

protected:
	/**
	 * GameInstance Subsystem 초기화 전에
	 * BP_SoundSubsystem 클래스를 메모리에 올리기 위한 하드 참조입니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Subsystem")
	TSubclassOf<UECSoundSubsystem> SoundSubsystemClass;
};
