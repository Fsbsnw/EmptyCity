#include "ECStaminaSet.h"
#include "AbilitySystem/Attribute/ECAttributeSet.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "GameplayEffectExtension.h"

UECStaminaSet::UECStaminaSet()
	: Stamina(100.0f)
	, MaxStamina(100.0f)
	, StaminaDrainRate(10.0f)
	, StaminaRegenRate(1.0f)
{
	bOutOfStamina = false;
	MaxStaminaBeforeAttributeChange = 0.0f;
	StaminaBeforeAttributeChange = 0.0f;
}

bool UECStaminaSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	// 수치 변화가 적용되기 전 값을 저장합니다.
	StaminaBeforeAttributeChange = GetStamina();
	MaxStaminaBeforeAttributeChange = GetMaxStamina();

	return true;
}

void UECStaminaSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	float MinimumStamina = 0.0f;

	// 효과에 대한 책임을 가지는 객체와 적용을 담당하는 객체를 추출합니다.
	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	if (Data.EvaluatedData.Attribute == GetStaminaCostAttribute())
	{
		// StaminaCost 값만큼 현재 스태미나를 감소시킨 후 보정합니다.
		SetStamina(FMath::Clamp(GetStamina() - GetStaminaCost(), MinimumStamina, GetMaxStamina()));
		SetStaminaCost(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaRecoveryAttribute())
	{
		// StaminaRecovery 값만큼 현재 스태미나를 증가시킨 후 보정합니다.
		SetStamina(FMath::Clamp(GetStamina() + GetStaminaRecovery(), MinimumStamina, GetMaxStamina()));
		SetStaminaRecovery(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), MinimumStamina, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxStaminaAttribute())
	{
		OnMaxStaminaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxStaminaBeforeAttributeChange, GetMaxStamina());
	}

	// 수치 변화 적용 전/후의 값이 동일하지 않은 경우에만 브로드캐스팅합니다.
	if (GetStamina() != StaminaBeforeAttributeChange)
	{
		OnStaminaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, StaminaBeforeAttributeChange, GetStamina());
	}

	// 스태미나 소진 브로드캐스팅은 단 한 번만 진행합니다.
	if ((GetStamina() <= 0.0f) && !bOutOfStamina)
	{
		OnOutOfStamina.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, StaminaBeforeAttributeChange, GetStamina());
	}

	// 스태미나 소진 상태를 갱신합니다.
	bOutOfStamina = (GetStamina() <= 0.0f);
}

void UECStaminaSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxStaminaAttribute())
	{
		// 최대 스태미나 값이 수정되었는데, 현재 스태미나가 해당 값보다 크다면 최대 스태미나 값에 맞게 보정합니다.
		if (GetStamina() > NewValue)
		{
			UECAbilitySystemComponent* ASC = GetAbilitySystemComponent();
			check(ASC);

			ASC->ApplyModToAttribute(GetStaminaAttribute(), EGameplayModOp::Override, NewValue);
		}
	}

	if (bOutOfStamina && (GetStamina() > 0.0f))
	{
		bOutOfStamina = false;
	}
}

void UECStaminaSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetStaminaDrainRateAttribute() || Attribute == GetStaminaRegenRateAttribute())
	{
		// Rate 어트리뷰트는 음수가 되지 않도록 보정합니다.
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}
