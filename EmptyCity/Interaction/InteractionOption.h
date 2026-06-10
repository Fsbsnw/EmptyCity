#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "InteractionOption.generated.h"

class IInteractableTarget;
class UUserWidget;

/**
 * FInteractionOption
 *
 * "하나의 상호작용 선택지"를 나타내는 구조체입니다.
 * 상호작용 대상 하나가 여러 개의 옵션을 제공할 수 있습니다(예: 상자 → "열기", "부수기").
 * 각 옵션은 UI에 표시할 텍스트와, 실제로 실행할 어빌리티 정보를 함께 가집니다.
 */
USTRUCT(BlueprintType)
struct FInteractionOption
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Interaction Info
// ─────────────────────────────────────────────────────────────
public:
	/** 이 옵션을 제공한 상호작용 대상(문, 상자 등)입니다. FInteractionOptionBuilder가 자동으로 채워줍니다. */
	UPROPERTY(BlueprintReadWrite)
	TScriptInterface<IInteractableTarget> InteractableTarget;

	/** UI에 표시할 메인 텍스트입니다 (예: "문 열기"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	/** UI에 표시할 보조 텍스트입니다 (예: "E 키를 누르세요"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SubText;

	
// ─────────────────────────────────────────────────────────────
// Interaction Option 1
// 플레이어(아바타)에게 어빌리티를 부여하고, 상호작용 시 플레이어가 직접 발동합니다.
// ─────────────────────────────────────────────────────────────
public:
	/** 상호작용 가능한 물체 근처에 갔을 때 플레이어에게 부여할 어빌리티 클래스입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> InteractionAbilityToGrant;

	
// ─────────────────────────────────────────────────────────────
// Interaction Option 2
// 상호작용 대상 물체가 자체 어빌리티 시스템과 어빌리티를 가지고 있어, 플레이어 대신 그 물체 쪽의 어빌리티를 발동합니다.
// ─────────────────────────────────────────────────────────────
public:
	/** 대상 물체가 가진 어빌리티 시스템 컴포넌트입니다. 아래 핸들로 어빌리티를 발동하거나 이벤트를 보낼 때 사용합니다. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystem = nullptr;

	/** 이 옵션에 대해 대상 물체에서 발동할 어빌리티의 핸들(식별자)입니다. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayAbilitySpecHandle TargetInteractionAbilityHandle;

	
// ─────────────────────────────────────────────────────────────
// Interaction UI
// ─────────────────────────────────────────────────────────────
public:
	/** 이 상호작용을 위해 화면에 띄울 위젯 클래스입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UUserWidget> InteractionWidgetClass;


// ─────────────────────────────────────────────────────────────
// Operator Overloading
// ─────────────────────────────────────────────────────────────
public:
	// 아래 비교 연산자들은 상호작용 옵션 목록이 "변했는지"를 판단하는 데 쓰입니다.
	// (예: 매 프레임 트레이스 결과를 이전 결과와 비교해, 바뀌었을 때만 UI를 갱신)

	// 모든 필드가 동일하면 같은 옵션으로 간주합니다.
	FORCEINLINE bool operator==(const FInteractionOption& Other) const
	{
		return InteractableTarget == Other.InteractableTarget &&
			InteractionAbilityToGrant == Other.InteractionAbilityToGrant&&
			TargetAbilitySystem == Other.TargetAbilitySystem &&
			TargetInteractionAbilityHandle == Other.TargetInteractionAbilityHandle &&
			InteractionWidgetClass == Other.InteractionWidgetClass &&
			Text.IdenticalTo(Other.Text) &&
			SubText.IdenticalTo(Other.SubText);
	}

	FORCEINLINE bool operator!=(const FInteractionOption& Other) const
	{
		return !operator==(Other);
	}

	// 옵션 목록을 정렬할 때 사용합니다. 대상 인터페이스의 포인터 주소를 기준으로 정렬해,
	// 두 목록을 순서대로 일관되게 비교할 수 있게 합니다.
	FORCEINLINE bool operator<(const FInteractionOption& Other) const
	{
		return InteractableTarget.GetInterface() < Other.InteractableTarget.GetInterface();
	}
};
