#include "ECPlayer.h"
#include "ECGameplayTags.h"
#include "GameplayTagContainer.h"
#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "AbilitySystem/Ability/Player/ECGameplayAbility_Sprint.h"
#include "AbilitySystem/Attribute/ECHealthSet.h"
#include "Camera/ECCameraComponent.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Controller/ECPlayerController.h"
#include "Equipment/ECEquipmentDefinition.h"
#include "Equipment/ECEquipmentInstance.h"
#include "Equipment/ECEquipmentManagerComponent.h"
#include "Equipment/ECQuickBarComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Input/ECInputComponent.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "Inventory/Fragment/InventoryFragment_EquippableItem.h"

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

	// 공격 Ability가 활성화된 동안 플레이어 이동 입력을 차단합니다.
	MovementBlockingTags.Add(ECGameplayTags::Status_Attacking.GetTag());
	
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

	// EquipmentManager를 초기화합니다.
	{
		EquipmentManagerComponent = CreateDefaultSubobject<UECEquipmentManagerComponent>(TEXT("EquipmentManagerComponent"));
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

	// 약공격과 강공격은 같은 좌클릭을 사용하지만 IMC의 Tap/Hold Trigger로 구분됩니다.
	// Started에서는 두 액션이 동시에 시작되므로, 각 Trigger의 판정이 끝난 Triggered 시점에만 Ability 입력을 전달합니다.
	IC->BindNativeAction(InputConfig, ECGameplayTags::InputTag_Ability_Attack_Light, ETriggerEvent::Triggered, this, &ThisClass::Input_AbilityInputTagPressed, ECGameplayTags::InputTag_Ability_Attack_Light.GetTag());
	IC->BindNativeAction(InputConfig, ECGameplayTags::InputTag_Ability_Attack_Heavy, ETriggerEvent::Triggered, this, &ThisClass::Input_AbilityInputTagPressed, ECGameplayTags::InputTag_Ability_Attack_Heavy.GetTag());
	
	IC->BindNativeAction(InputConfig, ECGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
	IC->BindNativeAction(InputConfig, ECGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse);

	// QuickBar 번호 키(1/2)를 슬롯 사용에 바인딩한다. 마지막 인자로 자신의 슬롯 인덱스를 핸들러에 전달한다.
	IC->BindNativeAction(InputConfig, ECGameplayTags::InputTag_QuickBar_Slot1, ETriggerEvent::Started, this, &ThisClass::Input_UseQuickSlot, 0);
	IC->BindNativeAction(InputConfig, ECGameplayTags::InputTag_QuickBar_Slot2, ETriggerEvent::Started, this, &ThisClass::Input_UseQuickSlot, 1);
}

void AECPlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	AECPlayerState* PS = GetPlayerState<AECPlayerState>();
	if (!PS)
	{
		return;
	}

	ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// 반드시 먼저 ActorInfo를 초기화한다.
	ASC->InitAbilityActorInfo(PS, this);

	// 그다음 GAS 몽타주 대상 메시를 지정한다.
	ASC->AbilityActorInfo->SkeletalMeshComponent = FirstPersonMesh;
}

void AECPlayer::BeginPlay()
{
	Super::BeginPlay();
	
	InventoryManagerComponent = GetController()->FindComponentByClass<UECInventoryManagerComponent>();
	QuickBarComponent = GetController()->FindComponentByClass<UECQuickBarComponent>();
	
	AddInitialInventory();
	EquipWeapon();
}

bool AECPlayer::CanJumpInternal_Implementation() const
{
	// Crouch 상태에서도 점프가 가능하도록, bIsCrouched 검사를 우회합니다.
	return JumpIsAllowedInternal();
}

void AECPlayer::AddInitialInventory()
{
	if (InitialInventoryItems.IsEmpty()) return;
	
	if (!InventoryManagerComponent.Get() || !QuickBarComponent.Get()) return;
	
	for (const FInitialInventoryItem& InitialItem : InitialInventoryItems)
	{
		if (!InitialItem.ItemDef) continue;

		// 인벤토리에 아이템을 지급한다.
		// @TODO: Inventory UI 구현시 ItemInstance를 UI에 추가할 것
		UECInventoryItemInstance* ItemInstance = InventoryManagerComponent->AddItemDefinition(InitialItem.ItemDef, InitialItem.StackCount);

		// QuickBar 등록이 켜져 있으면 지정 슬롯에도 등록한다.
		if (ItemInstance && InitialItem.bAddToQuickBar)
		{
			QuickBarComponent->AddItemToSlot(InitialItem.QuickBarSlot, ItemInstance);
		}
	}
}

void AECPlayer::EquipWeapon()
{
	if (!DefaultWeapon) return;
	
	if (UECInventoryItemInstance* WeaponInstance = InventoryManagerComponent->AddItemDefinition(DefaultWeapon, 1))
	{
		if (const UInventoryFragment_EquippableItem* EquipInfo = WeaponInstance->FindFragmentByClass<UInventoryFragment_EquippableItem>())
		{
			if (TSubclassOf<UECEquipmentDefinition> EquipDef = EquipInfo->EquipmentDefinition)
			{
				if (UECEquipmentInstance* EquippedWeapon = EquipmentManagerComponent->EquipItem(EquipInfo->EquipmentDefinition))
				{
					// GetWeaponData()가 원본 InventoryItem을 역추적할 수 있게 합니다.
					EquippedWeapon->Instigator = WeaponInstance;
				}
			}
		}	
	}
}

UECWeaponInstance* AECPlayer::GetEquippedWeapon() const
{
	return IsValid(EquipmentManagerComponent)
		? EquipmentManagerComponent->GetEquippedWeapon()
		: nullptr;
}

void AECPlayer::Input_Move(const FInputActionValue& InputActionValue)
{
	if (HasAnyMovementBlockingTag())
	{
		// 공중 속도는 유지하고 지상에서 남아 있는 이동 속도만 즉시 제거합니다.
		if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
			MovementComponent && MovementComponent->IsMovingOnGround())
		{
			MovementComponent->StopMovementImmediately();
		}

		return;
	}

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

bool AECPlayer::HasAnyMovementBlockingTag() const
{
	if (!ASC)
	{
		return false;
	}

	for (const FGameplayTag& BlockingTag : MovementBlockingTags)
	{
		if (BlockingTag.IsValid() && ASC->HasMatchingGameplayTag(BlockingTag))
		{
			return true;
		}
	}

	return false;
}

void AECPlayer::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	
	AddControllerYawInput(Value.X);
	AddControllerPitchInput(Value.Y);
}


void AECPlayer::Input_UseQuickSlot(const FInputActionValue& InputActionValue, int32 SlotIndex)
{
	if (!QuickBarComponent.Get()) return;
	
	// QuickBar는 PlayerController에 붙어 있으므로 Controller를 통해 접근한다.
	if (const AECPlayerController* PC = Cast<AECPlayerController>(GetController()))
	{
		QuickBarComponent->UseSlot(SlotIndex);
	}
}

void AECPlayer::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	ASC->AbilityInputTagPressed(InputTag);
}

void AECPlayer::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	ASC->AbilityInputTagReleased(InputTag);
}
