#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "ECAbilityCost.generated.h"

class UECGameplayAbility;

/**
 * UECAbilityCost
 *
 * ECGameplayAbility가 발동될 때 소비하는 비용(예: 탄약, 사용 횟수)을 정의하는 기본 클래스입니다.
 */
UCLASS(MinimalAPI, DefaultToInstanced, EditInlineNew, Abstract)
class UECAbilityCost : public UObject
{
	GENERATED_BODY()
    
// ─────────────────────────────────────────────────────────────
// AbilityCost Interface
// ─────────────────────────────────────────────────────────────
public:
	UECAbilityCost() { }
	
	/**
	 * 이 비용을 지불할 수 있는지 검사합니다.
	 * 실패 사유 태그를 OptionalRelevantTags(nullptr이 아닐 경우)에 추가할 수 있으며, 이 태그는 UI 피드백 등에서 활용됩니다.
	 * (예: 탄약이 부족하여 발사가 불가능할 때 클릭 사운드 재생)
	 * 호출 시점에 Ability와 ActorInfo는 nullptr이 아님이 보장되지만, OptionalRelevantTags는 nullptr일 수 있습니다.
	 * 비용을 지불할 수 있으면 true, 그렇지 않으면 false를 반환합니다.
	 */
	virtual bool CheckCost(const UECGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
	{
		return true;
	}

	/**
	 * 어빌리티의 비용을 대상에게 적용합니다.
	 * 호출 시점에 Ability와 ActorInfo는 nullptr이 아님이 보장됩니다.
	 */
	virtual void ApplyCost(const UECGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
	{
	}

};