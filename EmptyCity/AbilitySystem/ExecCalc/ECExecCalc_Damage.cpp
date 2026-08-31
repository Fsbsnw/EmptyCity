#include "ECExecCalc_Damage.h"

#include "ECGameplayTags.h"
#include "AbilitySystem/Attribute/ECCombatSet.h"
#include "AbilitySystem/Attribute/ECHealthSet.h"
#include "AbilitySystem/Attribute/ECStaminaSet.h"

struct FECDamageStatics
{
	// Source의 공격력
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);

	// Target의 스태미나
	DECLARE_ATTRIBUTE_CAPTUREDEF(Stamina);

	FECDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UECCombatSet, AttackPower, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UECStaminaSet, Stamina, Target, false);
	}
};

static const FECDamageStatics& DamageStatics()
{
	static FECDamageStatics Statics;
	return Statics;
}

UECExecCalc_Damage::UECExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().StaminaDef);
}

void UECExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvaluationParams;
	
	EvaluationParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// ─────────────────────────────────────────────
	// 가드 처리
	// ─────────────────────────────────────────────
    
	float CurrentStamina = 0.0f;
    
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().StaminaDef, EvaluationParams, CurrentStamina);
    
	CurrentStamina = FMath::Max(CurrentStamina, 0.0f);
    
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
    
	const bool bIsGuarding = TargetTags && TargetTags->HasTagExact(ECGameplayTags::Status_Guarding);
    
	// 가드 중이고 스태미나가 남아 있으면 체력 대신 스태미나에 피해를 줍니다.
	if (bIsGuarding && CurrentStamina > 0.0f)
	{
		const float StaminaDamage = Spec.GetSetByCallerMagnitude(ECGameplayTags::SetByCaller_StaminaDamage, false, 0.0f);
    
		if (StaminaDamage > 0.0f)
		{
			OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UECStaminaSet::GetStaminaCostAttribute(), EGameplayModOp::Additive, StaminaDamage));
		}
    
		return;
	}
	
	// ─────────────────────────────────────────────
	// 체력 데미지 처리
    // ─────────────────────────────────────────────
    
	// Source ASC가 보유한 기본 공격력을 가져옵니다.
	// 현재 구조에서는 플레이어의 힘 스탯 역할을 합니다.
	float SourceAttackPower = 0.0f;
    
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvaluationParams, SourceAttackPower);
    
	SourceAttackPower = FMath::Max(SourceAttackPower, 0.0f);
    
	// WeaponTraceComponent가 전달한 무기 공격력입니다.
	const float WeaponAttackPower = Spec.GetSetByCallerMagnitude(ECGameplayTags::SetByCaller_WeaponAttackPower, false, 0.0f);
    
	// 약공격 또는 강공격에 따른 공통 피해 배율입니다.
	const float DamageMultiplier = Spec.GetSetByCallerMagnitude(ECGameplayTags::SetByCaller_DamageMultiplier, false, 0.0f);
    
	// 최종 피해 = (플레이어 공격력 + 무기 공격력) × 공격 배율
	const float FinalDamage = FMath::Max((SourceAttackPower + WeaponAttackPower) * DamageMultiplier, 0.0f);
    
	if (FinalDamage <= 0.0f)
	{
		return;
	}
    
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UECHealthSet::GetDamageAttribute(), EGameplayModOp::Additive, FinalDamage));
	
	UE_LOG(LogTemp, Warning, TEXT("Damage: Source=%.1f Weapon=%.1f Multiplier=%.2f Final=%.1f"), SourceAttackPower, WeaponAttackPower, DamageMultiplier, FinalDamage);
}
