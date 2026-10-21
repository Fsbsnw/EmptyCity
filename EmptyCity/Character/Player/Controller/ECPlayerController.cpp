#include "ECPlayerController.h"

#include "ECGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "Framework/Application/SlateApplication.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "Character/Player/CameraManager/ECPlayerCameraManager.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Core/GameState/ECGameState.h"
#include "Data/UI/UIConfigDataAsset.h"
#include "Equipment/ECQuickBarComponent.h"
#include "Input/ECInputComponent.h"
#include "InputCoreTypes.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/ECGameplayMessageSubsystem.h"
#include "Subsystem/ECMessageTypes.h"
#include "Subsystem/ECTutorialSubsystem.h"
#include "UI/IndicatorSystem/ECIndicatorManagerComponent.h"
#include "UI/Subsystem/UIManagerSubsystem.h"
#include "UI/Widget/ECInvincibilityIndicatorWidget.h"

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

	// IndicatorManagerComponent를 초기화합니다.
	{
		IndicatorManagerComponent =	CreateDefaultSubobject<UECIndicatorManagerComponent>(TEXT("IndicatorManagerComponent"));
	}
}

UECInventoryManagerComponent* AECPlayerController::GetInventoryManagerComponent() const
{
	return InventoryManagerComponent;
}

UECIndicatorManagerComponent* AECPlayerController::GetIndicatorManagerComponent() const
{
	return IndicatorManagerComponent;
}

void AECPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (PlayerInputLockCount == 0)
	{
		if (const AECPlayerState* PS = GetPlayerState<AECPlayerState>())
		{
			if (UECAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
			{
				ASC->ProcessAbilityInput(DeltaTime,bGamePaused);
			}
		}
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void AECPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = false;

	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(false);
	SetInputMode(InputMode);

	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetAllUserFocusToGameViewport(EFocusCause::SetDirectly);
	}
	
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

bool AECPlayerController::InputKey(const FInputKeyParams& Params)
{
	if (Params.Key == EKeys::F5 && Params.Event == IE_Pressed)
	{
		Input_ToggleInvincibility();
		return true;
	}

	return Super::InputKey(Params);
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

	if (UECTutorialSubsystem* TutorialSubsystem =
		GetLocalPlayer()->GetSubsystem<UECTutorialSubsystem>())
	{
		TutorialSubsystem->InitializeTutorialUI();
	}
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

	FECTutorialInputMessage TutorialMessage;
	TutorialMessage.PlayerController = this;
	TutorialMessage.InputTag = InputTag;
	UECGameplayMessageSubsystem::Get(this).BroadcastMessage(
		TAG_Message_Tutorial_Input,
		TutorialMessage);
}

void AECPlayerController::Input_ToggleInvincibility()
{
	AECPlayerState* PS = GetPlayerState<AECPlayerState>();
	UECAbilitySystemComponent* ASC = PS ? PS->GetAbilitySystemComponent() : nullptr;
	if (!ASC)
	{
		return;
	}

	bInvincibilityEnabled = !ASC->HasMatchingGameplayTag(ECGameplayTags::Status_Invincible);
	ASC->SetLooseGameplayTagCount(
		ECGameplayTags::Status_Invincible,
		bInvincibilityEnabled ? 1 : 0);
	RefreshInvincibilityIndicator();
}

void AECPlayerController::RefreshInvincibilityIndicator()
{
	if (!bInvincibilityEnabled)
	{
		if (InvincibilityIndicatorWidget)
		{
			InvincibilityIndicatorWidget->RemoveFromParent();
			InvincibilityIndicatorWidget = nullptr;
		}
		return;
	}

	if (!InvincibilityIndicatorWidget)
	{
		InvincibilityIndicatorWidget = CreateWidget<UECInvincibilityIndicatorWidget>(this);
		if (InvincibilityIndicatorWidget)
		{
			InvincibilityIndicatorWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}

	if (InvincibilityIndicatorWidget && !InvincibilityIndicatorWidget->IsInViewport())
	{
		InvincibilityIndicatorWidget->AddToViewport(10000);
	}
}

void AECPlayerController::SetPlayerInputLocked(bool bIsLocked)
{
	if (bIsLocked)
	{
		++PlayerInputLockCount;
		if (PlayerInputLockCount > 1)
		{
			return;
		}

		DisableInput(this);
		if (APawn* MyPawn = GetPawn())
		{
			MyPawn->DisableInput(this);
		}

		if (const AECPlayerState* PS = GetPlayerState<AECPlayerState>())
		{
			if (UECAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
			{
				ASC->ClearAbilityInput();
			}
		}
	}
	else
	{
		if (PlayerInputLockCount == 0)
		{
			return;
		}

		--PlayerInputLockCount;
		if (PlayerInputLockCount > 0)
		{
			return;
		}

		EnableInput(this);
		if (APawn* MyPawn = GetPawn())
		{
			MyPawn->EnableInput(this);
		}
	}
}
