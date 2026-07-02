#pragma once

#include "CoreMinimal.h"
#include "Character/ECCharacterBase.h"
#include "ECPlayer.generated.h"

class UECInventoryItemDefinition;
class UECEquipmentManagerComponent;
class UECCameraComponent;
class UInputMappingContext;
class UECInputConfig;
struct FGameplayTag;
struct FInputActionValue;

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
	
	
// ─────────────────────────────────────────────────────────────	
// Inventory Function
// ─────────────────────────────────────────────────────────────	
private:
	/** 게임 시작 시 InitialInventoryItems에 설정된 기본 아이템들을 인벤토리에 채워 넣습니다. (BeginPlay에서 호출) */
	void AddInitialInventory();
	
	
// ─────────────────────────────────────────────────────────────
// Input Method
// ─────────────────────────────────────────────────────────────	
private:
	/** 플레이어 이동 입력에 따라 캐릭터를 움직이기 위한 바인딩하는 함수입니다. */
	void Input_Move(const FInputActionValue& InputActionValue);
	
	/** 플레이어의 마우스 움직임에 따라 카메라를 회전하기 위한 바인딩 함수입니다. */
	void Input_LookMouse(const FInputActionValue& InputActionValue);

	/** InputTag 입력에 따라 Ability를 발동시키기 위한 바인딩 함수입니다. */
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	
	/** InputTag 입력 종료에 따라 Ability를 중단하기 위한 바인딩 함수입니다. */
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	/** QuickBar 번호 키 입력에 따라 해당 슬롯의 아이템 사용을 요청하는 바인딩 함수입니다. (QuickBar는 Controller 소유이므로 Controller를 통해 접근) */
	void Input_UseQuickSlot(const FInputActionValue& InputActionValue, int32 SlotIndex);
	
	
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
	
	/** 이 캐릭터가 장착한 장비를 관리하는 컴포넌트입니다. (장비 Actor 스폰, 어빌리티 부여 등) */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UECEquipmentManagerComponent> EquipmentManagerComponent;

	
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
	
	
// ─────────────────────────────────────────────────────────────
// Inventory Variable
// ─────────────────────────────────────────────────────────────	
protected:
	/** 게임 시작 시 인벤토리에 기본으로 지급할 아이템 목록입니다. (항목별로 QuickBar 슬롯 등록 여부를 지정할 수 있습니다) */
	UPROPERTY(EditDefaultsOnly, Category = "변수|인벤토리")
	TArray<FInitialInventoryItem> InitialInventoryItems;
	
};
