#include "ECGameMode.h"
#include "Character/Player/ECPlayer.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Core/GameState/ECGameState.h"
#include "UI/HUD/ECHUD.h"

AECGameMode::AECGameMode()
{
	DefaultPawnClass = AECPlayer::StaticClass();
	PlayerControllerClass = AECPlayerController::StaticClass();
	PlayerStateClass = AECPlayerState::StaticClass();
	GameStateClass = AECGameState::StaticClass();
	HUDClass = AECHUD::StaticClass();
}
