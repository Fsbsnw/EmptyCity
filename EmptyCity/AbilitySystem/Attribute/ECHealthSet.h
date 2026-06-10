#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "ECAttributeSet.h"
#include "ECHealthSet.generated.h"

UCLASS()
class EMPTYCITY_API UECHealthSet : public UECAttributeSet
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// AttributeSet Interface
// ─────────────────────────────────────────────────────────────		
public:
	UECHealthSet();
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	ATTRIBUTE_ACCESSORS(UECHealthSet, Health);
	ATTRIBUTE_ACCESSORS(UECHealthSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UECHealthSet, Healing);
	ATTRIBUTE_ACCESSORS(UECHealthSet, Damage);

	
// ─────────────────────────────────────────────────────────────
// Delegate
// ─────────────────────────────────────────────────────────────	
public:	
	/** Damage, Healing으로 인한 현재 체력 변화 감지용 델리게이트입니다. */
	mutable FECAttributeEvent OnHealthChanged;

	/** 최대 체력 변화 감지용 델리게이트입니다. */
	mutable FECAttributeEvent OnMaxHealthChanged;

	/** 현재 체력이 0이하로 감소하는 순간을 감지하기 위한 델리게이트입니다. */
	mutable FECAttributeEvent OnOutOfHealth;
	
		
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
	/** 현재 체력 어트리뷰트입니다. 체력은 최대 체력 어트리뷰트 값으로 제한됩니다. 체력은 모디파이어로부터 숨겨져 있어 오직 Execution을 통해서만 수정할 수 있습니다. */
	UPROPERTY(BlueprintReadOnly, Category = "변수", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Health;

	/** 현재 최대 체력 어트리뷰트입니다. 게임플레이 이펙트로 수정할 수 있도록 어트리뷰트로 정의되어 있습니다. */
	UPROPERTY(BlueprintReadOnly, Category = "변수", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth;

	/** 체력이 0에 도달하는 시점을 추적하는 데 사용됩니다. */
	bool bOutOfHealth;

	/** 변경이 적용되기 전의 체력 값을 저장합니다. */
	float MaxHealthBeforeAttributeChange;
	float HealthBeforeAttributeChange;
	
	/** 들어오는 회복량입니다. +Health에 직접 매핑됩니다. */
	UPROPERTY(BlueprintReadOnly, Category="변수", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Healing;

	/** 들어오는 피해량입니다. -Health에 직접 매핑됩니다.*/
	UPROPERTY(BlueprintReadOnly, Category="변수", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Damage;
};
