#pragma once

#include "CoreMinimal.h"
#include "Character/ECCharacterBase.h"
#include "ECPlayer.generated.h"

class UECCameraComponent;
class UInputMappingContext;
class UECInputConfig;
struct FGameplayTag;
struct FInputActionValue;

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
	
};
