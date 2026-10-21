#include "ECPlayer.h"
#include "ECGameplayTags.h"
#include "GameplayTagContainer.h"
#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "AbilitySystem/Ability/Player/ECGameplayAbility_Sprint.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "AbilitySystem/Attribute/ECHealthSet.h"
#include "AbilitySystem/Attribute/ECMoveSpeedSet.h"
#include "Camera/ECCameraComponent.h"
#include "Character/Player/Component/ECFootstepComponent.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Component/Attribute/ECHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
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
#include "Subsystem/ECItemDataSubsystem.h"
#include "Subsystem/ECGameplayMessageSubsystem.h"
#include "Subsystem/ECMessageTypes.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogECPlayer, Log, All);

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

	// 도끼/곡괭이는 파밍 중에만 1인칭 Mesh 소켓에 표시합니다.
	{
		FarmingToolMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FarmingToolMeshComponent"));
		FarmingToolMeshComponent->SetupAttachment(FirstPersonMesh);
		FarmingToolMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FarmingToolMeshComponent->SetGenerateOverlapEvents(false);
		FarmingToolMeshComponent->SetOnlyOwnerSee(true);
		FarmingToolMeshComponent->SetCastShadow(false);
		FarmingToolMeshComponent->SetVisibility(false);
		FarmingToolMeshComponent->SetHiddenInGame(true);
	}

	// EquipmentManager를 초기화합니다.
	{
		EquipmentManagerComponent = CreateDefaultSubobject<UECEquipmentManagerComponent>(TEXT("EquipmentManagerComponent"));
	}

	// 이동 거리와 발밑 표면에 따라 발소리를 재생하는 컴포넌트를 생성합니다.
	FootstepComponent = CreateDefaultSubobject<UECFootstepComponent>(TEXT("FootstepComponent"));
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

	// 플레이어도 HealthSet의 체력 0 이벤트가 Death GameplayEvent로 이어지도록 연결합니다.
	HealthComponent->InitializeWithAbilitySystem(ASC);

	// 그다음 GAS 몽타주 대상 메시를 지정한다.
	ASC->AbilityActorInfo->SkeletalMeshComponent = FirstPersonMesh;

	// MoveSpeed Attribute 변경을 실제 이동 속도에 반영하고 현재 값으로 초기화합니다.
	const UECMoveSpeedSet* MoveSpeedSet = ASC->GetSet<UECMoveSpeedSet>();
	ASC->GetGameplayAttributeValueChangeDelegate(MoveSpeedSet->GetMoveSpeedAttribute()).AddUObject(this, &ThisClass::OnMoveSpeedChanged);
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeedSet->GetMoveSpeed();
}

void AECPlayer::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	// 달리기 중에는 SprintSpeed가 우선하며, 종료 시 최신 MoveSpeed가 반영됩니다.
	if (!ASC->HasMatchingGameplayTag(ECGameplayTags::Status_Sprinting))
	{
		GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
	}
}

void AECPlayer::BeginPlay()
{
	Super::BeginPlay();

	if (USkeletalMeshComponent* ThirdPersonMesh = GetMesh())
	{
		InitialMeshRelativeTransform = ThirdPersonMesh->GetRelativeTransform();
		InitialMeshCollisionProfileName = ThirdPersonMesh->GetCollisionProfileName();
		InitialMeshCollisionEnabled = ThirdPersonMesh->GetCollisionEnabled();
		bInitialMeshGravityEnabled = ThirdPersonMesh->IsGravityEnabled();
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		InitialCapsuleCollisionProfileName = Capsule->GetCollisionProfileName();
		InitialCapsuleCollisionEnabled = Capsule->GetCollisionEnabled();
	}

	if (CameraComponent)
	{
		InitialCameraRelativeTransform = CameraComponent->GetRelativeTransform();
	}

	if (FirstPersonMesh)
	{
		InitialFirstPersonCollisionEnabled = FirstPersonMesh->GetCollisionEnabled();
	}

	bDeathRecoveryStateCached = true;
	
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

void AECPlayer::StartDeathProcess()
{
	Super::StartDeathProcess();

	USkeletalMeshComponent* RagdollMesh = GetMesh();
	if (!IsValid(RagdollMesh) || !RagdollMesh->GetPhysicsAsset())
	{
		UE_LOG(LogECPlayer, Warning, TEXT("플레이어 사망 래그돌을 실행할 Physics Asset이 없습니다."));
		return;
	}

	// 공중 사망을 포함해 래그돌보다 먼저 캡슐과 캐릭터 이동을 끕니다.
	DisableMovementAndCollision();

	// 1인칭 팔과 파밍 도구는 숨기고 본체 Mesh만 래그돌로 사용합니다.
	if (FirstPersonMesh)
	{
		FirstPersonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FirstPersonMesh->SetHiddenInGame(true, true);
	}

	// 현재 시야 위치를 유지한 채 머리 본에 붙여 갑작스러운 카메라 점프를 방지합니다.
	if (CameraComponent && RagdollMesh->DoesSocketExist(DeathCameraSocketName))
	{
		CameraComponent->bUsePawnControlRotation = false;
		CameraComponent->AttachToComponent(
			RagdollMesh,
			FAttachmentTransformRules::KeepWorldTransform,
			DeathCameraSocketName);
	}
	else
	{
		UE_LOG(LogECPlayer, Warning,
			TEXT("플레이어 사망 카메라 본/소켓을 찾지 못했습니다: %s"),
			*DeathCameraSocketName.ToString());
	}

	RagdollMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	RagdollMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RagdollMesh->SetEnableGravity(true);
	RagdollMesh->SetAllBodiesBelowSimulatePhysics(
		DeathRagdollRootBoneName,
		true,
		true);
	RagdollMesh->SetAllBodiesBelowPhysicsBlendWeight(
		DeathRagdollRootBoneName,
		1.0f,
		false,
		true);
	RagdollMesh->WakeAllRigidBodies();
	RagdollMesh->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
	RagdollMesh->SetAllPhysicsLinearVelocity(FVector::ZeroVector, false);

	GetWorldTimerManager().SetTimer(
		DeathRagdollTimerHandle,
		this,
		&ThisClass::FinishDeathRagdoll,
		DeathRagdollDuration,
		false);
}

void AECPlayer::FinishDeathRagdoll()
{
	if (CameraComponent)
	{
		CameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}

	if (USkeletalMeshComponent* RagdollMesh = GetMesh(); IsValid(RagdollMesh))
	{
		RagdollMesh->PutAllRigidBodiesToSleep();
	}
}

void AECPlayer::RecoverFromDeath()
{
	GetWorldTimerManager().ClearTimer(DeathRagdollTimerHandle);

	if (USkeletalMeshComponent* RagdollMesh = GetMesh(); IsValid(RagdollMesh))
	{
		RagdollMesh->SetAllBodiesSimulatePhysics(false);
		RagdollMesh->SetAllBodiesPhysicsBlendWeight(0.0f, false);
		RagdollMesh->PutAllRigidBodiesToSleep();

		if (bDeathRecoveryStateCached)
		{
			RagdollMesh->SetCollisionProfileName(InitialMeshCollisionProfileName);
			RagdollMesh->SetCollisionEnabled(InitialMeshCollisionEnabled);
			RagdollMesh->SetEnableGravity(bInitialMeshGravityEnabled);
			RagdollMesh->SetRelativeTransform(InitialMeshRelativeTransform);
		}
		RagdollMesh->SetHiddenInGame(false, true);
	}

	if (CameraComponent)
	{
		CameraComponent->AttachToComponent(
			RootComponent,
			FAttachmentTransformRules::KeepRelativeTransform);
		if (bDeathRecoveryStateCached)
		{
			CameraComponent->SetRelativeTransform(InitialCameraRelativeTransform);
		}
		CameraComponent->bUsePawnControlRotation = true;
	}

	if (FirstPersonMesh)
	{
		FirstPersonMesh->SetCollisionEnabled(InitialFirstPersonCollisionEnabled);
		FirstPersonMesh->SetHiddenInGame(false, true);
	}

	SetFirstPersonFarmingToolActive(false);

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		if (bDeathRecoveryStateCached)
		{
			Capsule->SetCollisionProfileName(InitialCapsuleCollisionProfileName);
			Capsule->SetCollisionEnabled(InitialCapsuleCollisionEnabled);
		}
	}

	SetActorHiddenInGame(false);

	if (UCharacterMovementComponent* MoveComponent = GetCharacterMovement())
	{
		MoveComponent->StopMovementImmediately();
		MoveComponent->SetMovementMode(MOVE_Walking);
	}

	if (HealthComponent)
	{
		HealthComponent->ResetDeath();
	}

	if (ASC)
	{
		ASC->ReactivateOnSpawnAbilities();
	}
}

void AECPlayer::SetFirstPersonFarmingToolActive(bool bActive, EECWorkbenchUpgradeType ToolType)
{
	if (!IsValid(FirstPersonMesh) || !IsValid(FarmingToolMeshComponent))
	{
		return;
	}

	if (!bActive)
	{
		FarmingToolMeshComponent->SetVisibility(false);
		FarmingToolMeshComponent->SetHiddenInGame(true);
		FarmingToolMeshComponent->SetStaticMesh(nullptr);
		return;
	}

	const FECItemTableRow* ToolData = UECItemDataSubsystem::Get(this).FindFarmingToolData(ToolType);
	if (!ToolData || ToolData->WorldMesh.IsNull() || ToolData->FarmingAttachSocket.IsNone())
	{
		UE_LOG(LogECPlayer, Warning,
			TEXT("1인칭 파밍 도구 데이터가 불완전합니다. ToolType=%d, DT_Item의 WorldMesh/FarmingAttachSocket을 확인하세요."),
			static_cast<uint8>(ToolType));
		return;
	}

	FarmingToolMeshComponent->SetStaticMesh(ToolData->WorldMesh.LoadSynchronous());
	FarmingToolMeshComponent->AttachToComponent(
		FirstPersonMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		ToolData->FarmingAttachSocket);
	FarmingToolMeshComponent->SetRelativeTransform(ToolData->FarmingAttachTransform);
	FarmingToolMeshComponent->SetHiddenInGame(false);
	FarmingToolMeshComponent->SetVisibility(true);
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
		UECInventoryItemInstance* ItemInstance = InventoryManagerComponent->AddItemDefinition(
			InitialItem.ItemDef,
			InitialItem.StackCount,
			false);

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
	
	if (UECInventoryItemInstance* WeaponInstance = InventoryManagerComponent->AddItemDefinition(
		DefaultWeapon,
		1,
		false))
	{
		if (const UInventoryFragment_EquippableItem* EquipInfo = WeaponInstance->FindFragmentByClass<UInventoryFragment_EquippableItem>())
		{
			if (TSubclassOf<UECEquipmentDefinition> EquipDef = EquipInfo->EquipmentDefinition)
			{
				EquipmentManagerComponent->EquipItem(EquipInfo->EquipmentDefinition, WeaponInstance);
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
		if (!Value.IsNearlyZero())
		{
			FECTutorialInputMessage TutorialMessage;
			TutorialMessage.PlayerController = Cast<APlayerController>(Controller);
			TutorialMessage.InputTag = ECGameplayTags::InputTag_Move;
			UECGameplayMessageSubsystem::Get(this).BroadcastMessage(
				TAG_Message_Tutorial_Input,
				TutorialMessage);
		}

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
	FECTutorialInputMessage TutorialMessage;
	TutorialMessage.PlayerController = Cast<APlayerController>(Controller);
	TutorialMessage.InputTag = InputTag;
	UECGameplayMessageSubsystem::Get(this).BroadcastMessage(
		TAG_Message_Tutorial_Input,
		TutorialMessage);

	ASC->AbilityInputTagPressed(InputTag);
}

void AECPlayer::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	ASC->AbilityInputTagReleased(InputTag);
}
