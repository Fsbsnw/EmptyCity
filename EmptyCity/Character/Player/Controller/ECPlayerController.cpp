#include "ECPlayerController.h"

#include "ECGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "Character/Player/CameraManager/ECPlayerCameraManager.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Core/GameState/ECGameState.h"
#include "Data/UI/UIConfigDataAsset.h"
#include "Input/ECInputComponent.h"
#include "UI/Subsystem/UIManagerSubsystem.h"

AECPlayerController::AECPlayerController()
{
	PlayerCameraManagerClass = AECPlayerCameraManager::StaticClass();
}

void AECPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (const AECPlayerState* PS = GetPlayerState<AECPlayerState>())
	{
		if (UECAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->ProcessAbilityInput(DeltaTime,bGamePaused);
		}	
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void AECPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(UIInputMappingContext, 0);
	}

	UECInputComponent* IC = CastChecked<UECInputComponent>(InputComponent);
	
	IC->BindUIAction(UIInputConfig, this, &ThisClass::Input_UIAction);

	if (AECPlayerState* PS = GetPlayerState<AECPlayerState>())
	{
		// 초기화가 완료된 상태면 UI를 초기화합니다.
		if (PS->IsStatsInitialized())
		{
			InitializeUI();
		}
		// 초기화가 완료되지 않았다면 바인딩만 진행하고 나중에 UI를 초기화합니다.
		else
		{
			PS->OnStatsInitializedDelegate.AddUObject(this, &AECPlayerController::InitializeUI);
		}
	}
}

void AECPlayerController::InitializeUI()
{
	// 1. GameState에서 UI 데이터 에셋 확보
	UWorld* World = GetWorld();
	if (!World) return;

	AECGameState* GS = World->GetGameState<AECGameState>();
	if (!GS || !GS->UIConfigDataAsset) return;

	// 2. 확보한 데이터 에셋을 들고 각각의 전담 함수 호출
	UUIConfigDataAsset* ConfigAsset = GS->UIConfigDataAsset;

	// 3. UI Manager 초기화 호출
	InitializeUIManager(ConfigAsset);

	// 4. HUD 초기화
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UUIManagerSubsystem* UIManager = LP->GetSubsystem<UUIManagerSubsystem>())
		{
			UIManager->RouteUIInput(ECGameplayTags::InputTag_UI_HUD, this->GetPawn());
		}
	}
}

void AECPlayerController::InitializeUIManager(UUIConfigDataAsset* ConfigAsset)
{
	if (!ConfigAsset) return;

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return;

	UUIManagerSubsystem* UIManager = LP->GetSubsystem<UUIManagerSubsystem>();
	if (!UIManager) return;

	// UI 데이터 주입
	UIManager->InitializeUIConfig(ConfigAsset->UIConfigData);
}

void AECPlayerController::Input_UIAction(const FInputActionValue& Value, FGameplayTag InputTag)
{
	// 입력받은 Tag를 UIManagerSubsystem으로 바로 전달
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UUIManagerSubsystem* UIManager = LP->GetSubsystem<UUIManagerSubsystem>())
		{
			UIManager->RouteUIInputWithValue(InputTag, Value, this->GetPawn());
		}
	}
}