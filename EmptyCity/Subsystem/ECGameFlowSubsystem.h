#pragma once

#include "CoreMinimal.h"
#include "ECGameplayMessageSubsystem.h"
#include "ECTimeTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ECGameFlowSubsystem.generated.h"

class AECPlayer;

/**
 * 게임의 진행 흐름을 결정하는 Subsystem
 */
UCLASS()
class EMPTYCITY_API UECGameFlowSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

	/** 사망 어빌리티가 완전히 종료된 뒤 플레이어를 복구 지점으로 되돌립니다. */
	void CompletePlayerDeathFlow(AECPlayer* Player);

	/** 현재 표시 중인 플레이어 사망 흐름 데이터를 반환합니다. */
	const FECPlayerDeathMessage& GetCurrentPlayerDeathMessage() const { return CurrentPlayerDeathMessage; }

    /**
     * 아침 체크포인트를 저장해야 하는 시점에 호출한다.
     * 실제 SaveData 수집과 파일 저장은 SaveSubsystem이 담당한다.
     */
    bool SaveMorningCheckpoint();

private:
    /** 아침이 시작되면 현재 게임 상태를 체크포인트로 저장한다. */
    void HandleGameTimeChanged(int32 Day, EECTimeOfDay TimeOfDay);

	/** 광신도 본거지에서 보스 전투 구역 진입이 확정되면 해당 상태로 체크포인트를 갱신한다. */
	void HandleRegionChanged(FGameplayTag PreviousRegionTag, FGameplayTag NewRegionTag);

    void HandlePlayerDeath(FGameplayTag Channel, const FECPlayerDeathMessage& Message);
	void HandleBossHalfHealthReached(FGameplayTag Channel, const FECBossHalfHealthReachedMessage& Message);
	void HandleBossDeath(FGameplayTag Channel, const FECBossDeathMessage& Message);

	/** 버릴 수 있는 인벤토리 슬롯 하나를 무작위로 골라 전체 수량을 잃게 합니다. */
	void LoseRandomPlayerInventoryItem(FECPlayerDeathMessage& InOutMessage);

	/** 보스 전투 중 플레이어가 사망한 경우 실제 AI 보스의 체력을 최대치로 복구합니다. */
	void RestoreBossHealthForRetry();

    FECGameplayMessageListenerHandle PlayerDeathListenerHandle;
	FECGameplayMessageListenerHandle BossHalfHealthListenerHandle;
	FECGameplayMessageListenerHandle BossDeathListenerHandle;

    bool bDeathFlowInProgress = false;

	/** 사망 순간의 지역입니다. 지연 중 지역 태그가 바뀌어도 복귀 분기가 흔들리지 않게 보관합니다. */
	FGameplayTag DeathRegionTag;

	/** 사망 화면 ViewModel이 공격자와 분실 결과를 조회하는 현재 사망 데이터입니다. */
	UPROPERTY(Transient)
	FECPlayerDeathMessage CurrentPlayerDeathMessage;
};
