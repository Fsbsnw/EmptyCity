#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ECGameMode.generated.h"

UCLASS()
class EMPTYCITY_API AECGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// GameModeBase Interface
// ─────────────────────────────────────────────────────────────	
public:
	AECGameMode();
	virtual void BeginPlay() override;

private:
	/** 월드의 플레이어와 UI가 생성된 다음 대기 중인 디버그 시작 상태를 적용합니다. */
	void ApplyPendingDebugGameStart();
	
};


