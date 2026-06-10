#pragma once

#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_GrantNearbyInteraction.generated.h"

class UGameplayAbility;
class UObject;
struct FFrame;
struct FGameplayAbilitySpecHandle;
struct FObjectKey;

/**
 * UAbilityTask_GrantNearbyInteraction
 *
 * 어빌리티 태스크(AbilityTask)는 어빌리티가 살아 있는 동안 비동기로 무언가를 지속 수행하는 작업 단위입니다.
 *
 * 이 태스크는 일정 주기마다 플레이어 주변을 구체(Sphere)로 겹침 검사하여,
 * 근처에 있는 상호작용 대상들이 요구하는 어빌리티를 플레이어의 어빌리티 시스템에 미리 "부여(Grant)"합니다.
 * 즉, 플레이어가 상호작용 가능한 물체에 다가가면 그 상호작용 어빌리티를 쓸 수 있게 준비시키는 역할입니다.
 *
 * (참고: 화면에 표시할 옵션을 찾는 일은 UAbilityTask_WaitForInteractableTargets 쪽이 담당합니다.)
 */
UCLASS()
class UAbilityTask_GrantNearbyInteraction : public UAbilityTask
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// AbilityTask Interface
// ─────────────────────────────────────────────────────────────
public:
	/** 태스크가 시작될 때 호출됩니다. 주기적 검사 타이머를 켭니다. */
	virtual void Activate() override;
	
	/** 태스크가 끝날 때 호출됩니다. 타이머를 정리합니다. */
	virtual void OnDestroy(bool AbilityEnded) override;


// ─────────────────────────────────────────────────────────────
// Member Method
// ─────────────────────────────────────────────────────────────
public:	
	/**
	 * 이 태스크를 생성하는 블루프린트 노드용 정적 팩토리 함수입니다.
	 * 
	 * InteractionScanRange: 주변을 검사할 반경
	 * InteractionScanRate: 검사 주기(초)
	 * bShowDebug: true면 검사 범위(구체)와 검출된 대상을 화면에 디버그로 그립니다.
	 */
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_GrantNearbyInteraction* GrantAbilitiesForNearbyInteractors(UGameplayAbility* OwningAbility, float InteractionScanRange, float InteractionScanRate, bool bShowDebug = false);

private:
	/** 타이머에 의해 주기적으로 호출되어, 주변을 검사하고 필요한 어빌리티를 부여하는 핵심 함수입니다. */
	void QueryInteractables();
	
	
// ─────────────────────────────────────────────────────────────
// Member Variable
// ─────────────────────────────────────────────────────────────
private:
	/** 주변 검사 반경입니다. */
	float InteractionScanRange = 100;
	
	/** 검사 간격입니다. */
	float InteractionScanRate = 0.100;

	/** true면 검사 범위(구체)와 검출된 상호작용 대상을 화면에 디버그로 그립니다. */
	bool bShowDebug = true;

	/** QueryInteractables를 주기 호출하는 타이머 핸들입니다. */
	FTimerHandle QueryTimerHandle;

	/** 부여한 어빌리티를 기록해 두는 캐시입니다. 같은 어빌리티를 중복으로 부여하지 않도록 막아줍니다.
	* 
	* 키: 어빌리티 클래스, 값: 부여된 어빌리티 스펙 핸들. */
	TMap<FObjectKey, FGameplayAbilitySpecHandle> InteractionAbilityCache;
};