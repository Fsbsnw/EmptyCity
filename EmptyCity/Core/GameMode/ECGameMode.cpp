#include "ECGameMode.h"
#include "Character/Player/ECPlayer.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Core/GameInstance/ECGameInstance.h"
#include "Core/GameState/ECGameState.h"
#include "TimerManager.h"
#include "UI/HUD/ECHUD.h"

AECGameMode::AECGameMode()
{
	DefaultPawnClass = AECPlayer::StaticClass();
	PlayerControllerClass = AECPlayerController::StaticClass();
	PlayerStateClass = AECPlayerState::StaticClass();
	GameStateClass = AECGameState::StaticClass();
	HUDClass = AECHUD::StaticClass();
}

void AECGameMode::BeginPlay()
{
	Super::BeginPlay();

	// PlayerState 등 월드 객체가 초기화를 마친 다음 디버그 시간을 전달합니다.
	GetWorldTimerManager().SetTimerForNextTick(
		this,
		&ThisClass::ApplyPendingDebugGameStart);
}

void AECGameMode::ApplyPendingDebugGameStart()
{
	if (UECGameInstance* ECGameInstance = Cast<UECGameInstance>(GetGameInstance()))
	{
		ECGameInstance->ApplyPendingDebugGameStart();
	}
}
