#include "ECPlayer.h"
#include "ECGameplayTags.h"
#include "GameplayTagContainer.h"
#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "AbilitySystem/Ability/ECGameplayAbility_Sprint.h"
#include "AbilitySystem/Attribute/ECHealthSet.h"
#include "Camera/ECCameraComponent.h"
#include "Character/Player/State/ECPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Input/ECInputComponent.h"

AECPlayer::AECPlayer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// 캐릭터 기본 속성을 설정합니다.
	{
		bUseControllerRotationYaw = true;
	}
	
	// CharacterMovementComponent를 설정합니다.
	{
		GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	}
	
	// 1인칭 카메라를 생성합니다.
	{
		CameraComponent = CreateDefaultSubobject<UECCameraComponent>(TEXT("CameraComponent"));
		CameraComponent->SetupAttachment(RootComponent);
		CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
		CameraComponent->bUsePawnControlRotation = true;
	}
	
	// 1인칭 메시를 생성합니다.
	{
		FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
		FirstPersonMesh->SetupAttachment(CameraComponent);
		FirstPersonMesh->SetRelativeLocation(FVector(-10.0f, 0.0f, -150.0f));
		FirstPersonMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
		FirstPersonMesh->SetCastHiddenShadow(false);
	}
}

void AECPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	const APlayerController* PC = GetController<APlayerController>();
	check(PC);
	
	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);
	
	Subsystem->ClearAllMappings();

	for (const auto& SoftIMC : InputMappingContexts)
	{
		if (const UInputMappingContext* IMC = SoftIMC.LoadSynchronous())
		{
			// 키 재매핑이 가능하도록 등록합니다.
			if (UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings())
			{
				Settings->RegisterInputMappingContext(IMC);
			}
							
			FModifyContextOptions Options = {};
			Options.bIgnoreAllPressedKeysUntilRelease = false;
							
			// 우선순위가 높은 입력 매핑이 낮은 입력 매핑보다 우선적으로 처리됩니다.
			Subsystem->AddMappingContext(IMC, 0, Options);
		}
	}
	
	UECInputComponent* IC = CastChecked<UECInputComponent>(PlayerInputComponent);
	
	IC->BindAbilityAction(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased);
	
	IC->BindNativeAction(InputConfig, ECGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
	IC->BindNativeAction(InputConfig, ECGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse);

}

void AECPlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	ASC = GetPlayerState<AECPlayerState>()->GetAbilitySystemComponent();
}

void AECPlayer::BeginPlay()
{
	Super::BeginPlay();

	if (ASC)
	{
		if (const UECHealthSet* HealthSet = ASC->GetSet<UECHealthSet>())
		{
			UE_LOG(LogTemp, Log, TEXT("[ECPlayer] Health: %.2f / MaxHealth: %.2f"),
				HealthSet->GetHealth(), HealthSet->GetMaxHealth());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ECPlayer] BeginPlay: HealthSet not found on ASC."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ECPlayer] BeginPlay: ASC is null."));
	}
}

bool AECPlayer::CanJumpInternal_Implementation() const
{
	// Crouch 상태에서도 점프가 가능하도록, bIsCrouched 검사를 우회합니다.
	return JumpIsAllowedInternal();
}

void AECPlayer::Input_Move(const FInputActionValue& InputActionValue)
{
	if (Controller)
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void AECPlayer::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	
	AddControllerYawInput(Value.X);
	AddControllerPitchInput(Value.Y);
}

void AECPlayer::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	ASC->AbilityInputTagPressed(InputTag);
}

void AECPlayer::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	ASC->AbilityInputTagReleased(InputTag);
}
