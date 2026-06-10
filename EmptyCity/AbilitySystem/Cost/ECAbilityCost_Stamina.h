#pragma once

#include "CoreMinimal.h"
#include "ECAbilityCost.h"
#include "ECAbilityCost_Stamina.generated.h"

UCLASS()
class EMPTYCITY_API UECAbilityCost_Stamina : public UECAbilityCost
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// AbilityCost Interface
// ─────────────────────────────────────────────────────────────	
public:
	UECAbilityCost_Stamina();
	virtual bool CheckCost(const UECGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const UECGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	
// ─────────────────────────────────────────────────────────────
// AbilityCost Variable
// ─────────────────────────────────────────────────────────────
protected:
	/** 소모할 체력량 (어빌리티 레벨로 스케일) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="변수")
	FScalableFloat Amount;

	/** 코스트 지불 시점에 처리할 GE (StaminaCost 모디파이어를 가진 GE 지정) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="변수")
	TSubclassOf<UGameplayEffect> CostEffect;

	/** 비용 부족 시 OptionalRelevantTags에 추가할 실패 사유 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="변수")
	FGameplayTag FailureTag;
	
};
