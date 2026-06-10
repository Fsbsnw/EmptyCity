#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Ability/ECGameplayAbility.h"
#include "ECAbilitySystemComponent.generated.h"

class UECGameplayAbility;

UCLASS()
class EMPTYCITY_API UECAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// AbilitySystemComponent Interface
// ─────────────────────────────────────────────────────────────
public:
	UECAbilitySystemComponent();
	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
	
	
// ─────────────────────────────────────────────────────────────
// Ability Input Section
// ─────────────────────────────────────────────────────────────		
public:	
	/** 입력 키가 눌린 프레임에 호출. 실제 발동은 하지 않고 처리 대기 목록에만 등록합니다. */
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	
	/** 입력 키에서 손을 뗀 프레임에 호출. "유지" 목록에서 빼고 "떼짐" 목록에 등록한다. */
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	
	/** 매 프레임 호출되어, 위에서 모아둔 입력 목록을 실제 Ability 발동으로 변환한다. */
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

private:	
	/** 입력이 시작된 Ability를 관리하는 Handle 입니다. */
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	
	/** 입력이 끝난 Ability를 관리하는 Handle 입니다. */
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
	
	/** 입력 중인 Ability를 관리하는 Handle 입니다. */
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
	
	
// ─────────────────────────────────────────────────────────────
// Ability Group Section
// ─────────────────────────────────────────────────────────────
public:
	/**
	 * 새 어빌리티가 해당 Group으로 발동 가능한 상태인지 검사합니다.
	 * Independent는 항상 통과, Exclusive_Replaceable/Blocking은 현재 활성 중인 Exclusive_Blocking이 있으면 차단됩니다.
	 */
	bool IsActivationGroupBlocked(EAbilityActivationGroup Group) const;

	/**
	 * 어빌리티 발동 시점에 호출되어 Group의 활성 카운트를 1 증가시킵니다.
	 * Exclusive 계열(Replaceable/Blocking) 어빌리티가 진입하는 경우, 기존에 활성 중이던 모든 Exclusive_Replaceable 어빌리티를 자동으로 취소합니다 (Ability 자신은 제외).
	 */
	void AddAbilityToActivationGroup(EAbilityActivationGroup Group, UECGameplayAbility* Ability);

	/** 어빌리티 종료 시점에 호출되어 Group의 활성 카운트를 1 감소시킵니다. */
	void RemoveAbilityFromActivationGroup(EAbilityActivationGroup Group, UECGameplayAbility* Ability);

	/** 지정한 Group에 속한 모든 활성 어빌리티를 취소합니다. IgnoreAbility는 취소 대상에서 제외됩니다. */
	void CancelActivationGroupAbilities(EAbilityActivationGroup Group, UECGameplayAbility* IgnoreAbility);
	
	/**
	 * 활성 어빌리티 인스턴스들을 순회하며 ShouldCancelFunc가 true를 반환한 것만 취소하는 일반 엔진입니다.
	 * CanBeCanceled가 false인 어빌리티는 취소하지 않고 로그만 남깁니다.
	 * 새로운 취소 정책이 필요한 경우, 이 함수를 감싸는 얇은 wrapper로 구현하세요 (예: CancelActivationGroupAbilities).
	 */
	typedef TFunctionRef<bool(const UECGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
	void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc);
	
private:	
	// 각 그룹마다 발동되어 있는 어빌리티를 관리하는 변수입니다. */
	int32 ActivationGroupCounts[(uint8)EAbilityActivationGroup::MAX];
	
};
