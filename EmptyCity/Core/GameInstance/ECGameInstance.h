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

	/**
	 * 세이브 파일을 사용하지 않고 LV_Main을 8일차 오전 상태로 시작합니다.
	 * 메인 메뉴와 인게임 시스템 메뉴가 같은 진입점을 사용합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Debug", meta = (DevelopmentOnly, DisplayName = "Debug Start Day 8 Morning"))
	void StartDay8MorningDebugGame();

	/** 새 월드가 준비된 뒤 대기 중인 디버그 시작 상태를 적용합니다. */
	bool ApplyPendingDebugGameStart();

protected:
	/**
	 * GameInstance Subsystem 초기화 전에
	 * BP_SoundSubsystem 클래스를 메모리에 올리기 위한 하드 참조입니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Subsystem")
	TSubclassOf<UECSoundSubsystem> SoundSubsystemClass;

private:
	/** 맵 전환 뒤 8일차 오전 상태를 적용해야 하는지 여부입니다. */
	bool bPendingDay8MorningDebugStart = false;
};
