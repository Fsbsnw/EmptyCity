#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "ECAttributeSet.h"
#include "ECStaminaSet.generated.h"

UCLASS()
class EMPTYCITY_API UECStaminaSet : public UECAttributeSet
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// AttributeSet Interface
// ─────────────────────────────────────────────────────────────
public:
	UECStaminaSet();
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	ATTRIBUTE_ACCESSORS(UECStaminaSet, Stamina);
	ATTRIBUTE_ACCESSORS(UECStaminaSet, MaxStamina);
	ATTRIBUTE_ACCESSORS(UECStaminaSet, StaminaRecovery);
	ATTRIBUTE_ACCESSORS(UECStaminaSet, StaminaCost);
	ATTRIBUTE_ACCESSORS(UECStaminaSet, StaminaDrainRate);
	ATTRIBUTE_ACCESSORS(UECStaminaSet, StaminaRegenRate);


// ─────────────────────────────────────────────────────────────
// Delegate
// ─────────────────────────────────────────────────────────────
public:
	/** StaminaCost, StaminaRecovery로 인한 현재 스태미나 변화 감지용 델리게이트입니다. */
	mutable FECAttributeEvent OnStaminaChanged;

	/** 최대 스태미나 변화 감지용 델리게이트입니다. */
	mutable FECAttributeEvent OnMaxStaminaChanged;

	/** 현재 스태미나가 0이하로 감소하는 순간을 감지하기 위한 델리게이트입니다. */
	mutable FECAttributeEvent OnOutOfStamina;


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
	/** 현재 스태미나 어트리뷰트입니다. 스태미나는 최대 스태미나 어트리뷰트 값으로 제한됩니다. 스태미나는 모디파이어로부터 숨겨져 있어 Recovery, Cost을 통해서만 수정할 수 있습니다. */
	UPROPERTY(BlueprintReadOnly, Category = "변수", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Stamina;

	/** 현재 최대 스태미나 어트리뷰트입니다. 게임플레이 이펙트로 수정할 수 있도록 어트리뷰트로 정의되어 있습니다. */
	UPROPERTY(BlueprintReadOnly, Category = "변수", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxStamina;

	/** 스태미나가 0에 도달하는 시점을 추적하는 데 사용됩니다. */
	bool bOutOfStamina;

	/** 변경이 적용되기 전의 스태미나 값을 저장합니다. */
	float MaxStaminaBeforeAttributeChange;
	float StaminaBeforeAttributeChange;

	/** 들어오는 회복량입니다. +Stamina에 직접 매핑됩니다. */
	UPROPERTY(BlueprintReadOnly, Category="변수", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData StaminaRecovery;

	/** 들어오는 소모량입니다. -Stamina에 직접 매핑됩니다. */
	UPROPERTY(BlueprintReadOnly, Category="변수", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData StaminaCost;

	/** Periodic GE의 소모(Drain)량을 결정하는 영속 어트리뷰트입니다. 무기/디버프 등 외부 GE가 수정합니다. */
	UPROPERTY(BlueprintReadOnly, Category="변수", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData StaminaDrainRate;

	/** Periodic GE의 회복(Regen)량을 결정하는 영속 어트리뷰트입니다. 버프/패시브 등 외부 GE가 수정합니다. */
	UPROPERTY(BlueprintReadOnly, Category="변수", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData StaminaRegenRate;
	
};
