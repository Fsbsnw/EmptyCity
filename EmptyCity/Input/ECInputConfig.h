#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ECInputConfig.generated.h"

class UInputAction;
struct FGameplayTag;

/** GameplayTag와 InputAction을 연결하는 래퍼 클래스입니다. */
USTRUCT(BlueprintType)
struct FECInputAction
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

UCLASS()
class EMPTYCITY_API UECInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// DataAsset Interface
// ─────────────────────────────────────────────────────────────	
public:
	UECInputConfig(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	
// ─────────────────────────────────────────────────────────────
// InputAction Finding
// ─────────────────────────────────────────────────────────────	
public:	
	/** 태그에 해당하는 플레이어 이동, 달리기 등과 같은 기본 입력 액션을 반환하는 함수입니다. */
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag) const;
	
	/** 태그에 해당하는 공격, 점프 등과 같은 어빌리티 입력 액션을 반환하는 함수입니다. */
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag) const;

	/** 태그에 해당하는 인벤토리, 시스템 메뉴 등과 같은 UI 조작 관련 입력 액션을 반환하는 함수입니다. */
	const UInputAction* FindUIInputActionForTag(const FGameplayTag& InputTag) const;

	
	
// ─────────────────────────────────────────────────────────────
// InputAction Variable
// ─────────────────────────────────────────────────────────────	
public:
	/** 플레이어 기본 입력 액션입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "InputAction", AllowPrivateAccess = true))
	TArray<FECInputAction> NativeInputActions;
	
	/** 어빌리티 입력 액션입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "InputAction", AllowPrivateAccess = true))
	TArray<FECInputAction> AbilityInputActions;

	/** UI 입력 액션입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "InputAction", AllowPrivateAccess = true))
	TArray<FECInputAction> UIInputActions;
	
};