#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "ECAttributeSet.h"
#include "ECMoveSpeedSet.generated.h"

UCLASS()
class EMPTYCITY_API UECMoveSpeedSet : public UECAttributeSet
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// AttributeSet Interface
// ─────────────────────────────────────────────────────────────
public:
	UECMoveSpeedSet();

	ATTRIBUTE_ACCESSORS(UECMoveSpeedSet, MoveSpeed);
	ATTRIBUTE_ACCESSORS(UECMoveSpeedSet, SprintSpeed);


// ─────────────────────────────────────────────────────────────
// Clamp Method
// ─────────────────────────────────────────────────────────────
protected:
	/** Attribute 값을 보정하는 함수입니다. */
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const override;


// ─────────────────────────────────────────────────────────────
// Attribute Variable
// ─────────────────────────────────────────────────────────────
private:
	/** 기본 이동 속도 어트리뷰트입니다. CharacterMovementComponent의 MaxWalkSpeed로 사용됩니다. */
	UPROPERTY(BlueprintReadOnly, Category = "변수", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MoveSpeed;

	/** 달리기 이동 속도 어트리뷰트입니다. Sprint 어빌리티 활성 시 MaxWalkSpeed로 사용됩니다. */
	UPROPERTY(BlueprintReadOnly, Category = "변수", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData SprintSpeed;
};
