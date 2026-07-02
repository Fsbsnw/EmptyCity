#include "ECPlayerController.h"

#include "ECGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "Character/Player/CameraManager/ECPlayerCameraManager.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Core/GameState/ECGameState.h"
#include "Data/UI/UIConfigDataAsset.h"
#include "Equipment/ECQuickBarComponent.h"
#include "Input/ECInputComponent.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Subsystem/UIManagerSubsystem.h"

AECPlayerController::AECPlayerController()
{
	PlayerCameraManagerClass = AECPlayerCameraManager::StaticClass();
	
	// InventoryManager를 초기화합니다.
	{
		InventoryManagerComponent = CreateDefaultSubobject<UECInventoryManagerComponent>(TEXT("InventoryManagerComponent"));
	}
	
	// QuickBarComponent를 초기화합니다.
	{
		QuickBarComponent = CreateDefaultSubobject<UECQuickBarComponent>(TEXT("QuickBarComponent"));
	}
}

UECInventoryManagerComponent* AECPlayerController::GetInventoryManagerComponent() const
{
	return InventoryManagerComponent;
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
	UWorld* World = GetWorld();
	if (!World) return;

	AECGameState* GS = World->GetGameState<AECGameState>();
	if (!GS || !GS->UIConfigDataAsset) return;

	// 1. GameState에서 UI 데이터 에셋 확보
	UUIConfigDataAsset* ConfigAsset = GS->UIConfigDataAsset;
	if (!ConfigAsset) return;

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return;

	UUIManagerSubsystem* UIManager = LP->GetSubsystem<UUIManagerSubsystem>();
	if (!UIManager) return;

	// 2. UI 데이터 설정 주입
	UIManager->InitializeUIConfig(ConfigAsset->UIConfigData);

	// 3. HUD 초기화
	UIManager->RequestToggleInput(ECGameplayTags::InputTag_UI_HUD, this->GetPawn());
}

void AECPlayerController::Input_UIAction(FGameplayTag InputTag)
{
	// 입력받은 Tag를 UIManagerSubsystem으로 바로 전달
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UUIManagerSubsystem* UIManager = LP->GetSubsystem<UUIManagerSubsystem>())
		{
			UIManager->RequestToggleInput(InputTag, this->GetPawn());
		}
	}
}

void AECPlayerController::SetCinematicInputLocked(bool bIsLocked)
{
	if (bIsLocked)
	{
		this->DisableInput(this);
		if (APawn* MyPawn = GetPawn()) MyPawn->DisableInput(this);
	}
	else
	{
		this->EnableInput(this);
		if (APawn* MyPawn = GetPawn()) MyPawn->EnableInput(this);
	}
	// SetCinematicMode()
}