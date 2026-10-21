#pragma once

#include "CoreMinimal.h"
#include "Character/ECCharacterBase.h"
#include "Data/Workbench/ECWorkbenchUpgradeTypes.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "ECPlayer.generated.h"

class UECQuickBarComponent;
class UECFootstepComponent;
class UECInventoryManagerComponent;
class UECInventoryItemDefinition;
class UECEquipmentManagerComponent;
class UECWeaponInstance;
class UECCameraComponent;
class UStaticMeshComponent;
class UInputMappingContext;
class UECInputConfig;
struct FInputActionValue;
struct FOnAttributeChangeData;

/** 게임 시작 시 지급할 초기 아이템 한 개의 설정입니다. (인벤토리 지급 + 선택적 QuickBar 등록) */
USTRUCT(BlueprintType)
struct FInitialInventoryItem
{
	GENERATED_BODY()

	/** 지급할 아이템 설계도입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수")
	TSubclassOf<UECInventoryItemDefinition> ItemDef;

	/** 지급할 개수입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수", meta = (ClampMin = "1"))
	int32 StackCount = 1;

	/** 이 아이템을 QuickBar 슬롯에도 등록할지 여부입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수")
	bool bAddToQuickBar = false;

	/** 등록할 QuickBar 슬롯 인덱스입니다. (bAddToQuickBar가 true일 때만 입력 가능) */
	UPROPERTY(EditDefaultsOnly, Category = "변수", meta = (EditCondition = "bAddToQuickBar", ClampMin = "0"))
	int32 QuickBarSlot = 0;
};

UCLASS()
class EMPTYCITY_API AECPlayer : public AECCharacterBase
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// Character Interface
// ─────────────────────────────────────────────────────────────	
public:
	AECPlayer(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;
	virtual bool CanJumpInternal_Implementation() const override;

protected:
	/** 플레이어 본체를 래그돌로 전환하고 카메라가 머리 본을 따라가게 합니다. */
	virtual void StartDeathProcess() override;

	/** 래그돌을 멈추고 카메라를 마지막 월드 위치에 유지합니다. */
	void FinishDeathRagdoll();
	
	
// ─────────────────────────────────────────────────────────────
// Attribute Method
// ─────────────────────────────────────────────────────────────
private:
	/** MoveSpeed Attribute 변경값을 CharacterMovementComponent에 반영합니다. */
	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);


// ─────────────────────────────────────────────────────────────
// Inventory Function
// ─────────────────────────────────────────────────────────────	
private:
	/** 게임 시작 시 InitialInventoryItems에 설정된 기본 아이템들을 인벤토리에 채워 넣습니다. (BeginPlay에서 호출) */
	void AddInitialInventory();
	
	/** 게임 시작 시 DefaultWeapon으로 지정한 무기를 장착합니다. */
	void EquipWeapon();
	
	
// ─────────────────────────────────────────────────────────────
// Input Method
// ─────────────────────────────────────────────────────────────	
private:
	/** 플레이어 이동 입력에 따라 캐릭터를 움직이기 위한 바인딩하는 함수입니다. */
	void Input_Move(const FInputActionValue& InputActionValue);
	
	/** 플레이어의 마우스 움직임에 따라 카메라를 회전하기 위한 바인딩 함수입니다. */
	void Input_LookMouse(const FInputActionValue& InputActionValue);
	
	/** QuickBar 번호 키 입력에 따라 해당 슬롯의 아이템 사용을 요청하는 바인딩 함수입니다. (QuickBar는 Controller 소유이므로 Controller를 통해 접근) */
    void Input_UseQuickSlot(const FInputActionValue& InputActionValue, int32 SlotIndex);

	/** InputTag 입력에 따라 Ability를 발동시키기 위한 바인딩 함수입니다. */
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	
	/** InputTag 입력 종료에 따라 Ability를 중단하기 위한 바인딩 함수입니다. */
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	
		
// ─────────────────────────────────────────────────────────────
// Tag Query
// ─────────────────────────────────────────────────────────────
private:
	/** ASC가 MovementBlockingTags 중 하나라도 보유하고 있는지 확인합니다. */
	bool HasAnyMovementBlockingTag() const;


// ───────────────────────────────────────────────────────────
// Farming Animation
// ───────────────────────────────────────────────────────────
public:
	/** 파밍 대상에 맞는 도끼/곡괭이를 1인칭 Mesh 소켓에 표시하거나 숨깁니다. */
	void SetFirstPersonFarmingToolActive(bool bActive, EECWorkbenchUpgradeType ToolType = EECWorkbenchUpgradeType::MAX);

	/** 현재 파밍 대상의 DT_Sound 행을 설정합니다. */
	void SetActiveFarmingSound(const FDataTableRowHandle& InFarmingSound) { ActiveFarmingSound = InFarmingSound; }

	/** 현재 파밍 대상의 DT_Sound 행을 반환합니다. */
	const FDataTableRowHandle& GetActiveFarmingSound() const { return ActiveFarmingSound; }

	/** 사망 연출을 종료하고 현재 플레이어 Pawn을 다시 조작 가능한 상태로 복구합니다. */
	void RecoverFromDeath();
	
	
	
// ─────────────────────────────────────────────────────────────
// Component
// ─────────────────────────────────────────────────────────────	
protected:
	/** 1인칭 시점을 렌더링할 카메라 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UECCameraComponent> CameraComponent;
	
	/** 1인칭 시점의 Mesh 입니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

	/** DT_Item의 도끼/곡괭이 메시를 파밍 중에만 표시하는 1인칭 전용 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, Category = "변수|파밍")
	TObjectPtr<UStaticMeshComponent> FarmingToolMeshComponent;

	/** 이 캐릭터가 장착한 장비를 관리하는 컴포넌트입니다. (장비 Actor 스폰, 어빌리티 부여 등) */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UECEquipmentManagerComponent> EquipmentManagerComponent;

	/** 이동 거리와 발밑 표면에 따라 발소리를 재생하는 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UECFootstepComponent> FootstepComponent;


// ─────────────────────────────────────────────────────────────
// Death Ragdoll
// ─────────────────────────────────────────────────────────────
protected:
	/** 사망 카메라가 따라갈 3인칭 Mesh의 본 또는 소켓입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|사망|래그돌")
	FName DeathCameraSocketName = TEXT("Head");

	/** 래그돌 시뮬레이션과 충격을 시작할 본입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|사망|래그돌")
	FName DeathRagdollRootBoneName = TEXT("pelvis");

	/** 래그돌을 유지할 시간입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|사망|래그돌", meta = (ClampMin = "0.0"))
	float DeathRagdollDuration = 2.0f;

	FTimerHandle DeathRagdollTimerHandle;

	/** 사망 래그돌 해제 후 복원할 컴포넌트 초기 상태입니다. */
	FTransform InitialMeshRelativeTransform;
	FTransform InitialCameraRelativeTransform;
	FName InitialMeshCollisionProfileName;
	FName InitialCapsuleCollisionProfileName;
	ECollisionEnabled::Type InitialMeshCollisionEnabled = ECollisionEnabled::NoCollision;
	ECollisionEnabled::Type InitialCapsuleCollisionEnabled = ECollisionEnabled::QueryAndPhysics;
	ECollisionEnabled::Type InitialFirstPersonCollisionEnabled = ECollisionEnabled::NoCollision;
	bool bInitialMeshGravityEnabled = true;
	bool bDeathRecoveryStateCached = false;

	
// ─────────────────────────────────────────────────────────────
// Input Variable
// ─────────────────────────────────────────────────────────────	
protected:
	/** 플레이어에게 부여할 입력 액션입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|입력")
	TObjectPtr<UECInputConfig> InputConfig;
	
	/** 플레이어에게 부여할 InputMappingContext입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|입력")
	TArray<TSoftObjectPtr<UInputMappingContext>> InputMappingContexts;

	/** 플레이어가 하나라도 보유하고 있으면 이동 입력을 차단하는 GameplayTag 목록입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "변수|입력|이동 차단", meta = (Categories = "Status"))
	TArray<FGameplayTag> MovementBlockingTags;
	
	
// ─────────────────────────────────────────────────────────────
// Inventory Variable
// ─────────────────────────────────────────────────────────────	
protected:
	/** 게임 시작 시 인벤토리에 기본으로 지급할 아이템 목록입니다. (항목별로 QuickBar 슬롯 등록 여부를 지정할 수 있습니다) */
	UPROPERTY(EditDefaultsOnly, Category = "변수|인벤토리")
	TArray<FInitialInventoryItem> InitialInventoryItems;
	
	/** 기본으로 장착할 무기입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|무기")
	TSubclassOf<UECInventoryItemDefinition> DefaultWeapon;
	
	
	
// ─────────────────────────────────────────────────────────────
// Getter
// ─────────────────────────────────────────────────────────────
public:
	FORCEINLINE USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }
	UECWeaponInstance* GetEquippedWeapon() const;
	
	
	
// ─────────────────────────────────────────────────────────────
// Cached Variable
// ─────────────────────────────────────────────────────────────
private:
	TWeakObjectPtr<UECInventoryManagerComponent> InventoryManagerComponent;
	TWeakObjectPtr<UECQuickBarComponent> QuickBarComponent;

	/** 공용 파밍 몽타주의 Notify가 사용할 현재 파밍 대상별 사운드입니다. */
	FDataTableRowHandle ActiveFarmingSound;
};
