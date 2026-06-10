#pragma once

#include "Interaction/InteractionQuery.h"
#include "Interaction/Task/AbilityTask_WaitForInteractableTargets.h"

#include "AbilityTask_WaitForInteractableTargets_SingleLineTrace.generated.h"

struct FCollisionProfileName;

class UGameplayAbility;
class UObject;
struct FFrame;

/**
 * UAbilityTask_WaitForInteractableTargets_SingleLineTrace
 *
 * UAbilityTask_WaitForInteractableTargets의 구체 구현 중 하나입니다.
 * 일정 주기마다 플레이어가 조준하는 방향으로 라인 트레이스(직선 한 줄)를 쏴서,
 * 그 선에 맞은 물체가 상호작용 가능한지 확인하고 옵션 목록을 갱신합니다.
 *
 * 즉, "조준한 물체와 상호작용" 방식의 탐지를 담당합니다(예: FPS에서 정조준한 문).
 * 이 태스크는 어빌리티가 끝날 때까지 스스로 반복 실행됩니다.
 */
UCLASS()
class UAbilityTask_WaitForInteractableTargets_SingleLineTrace : public UAbilityTask_WaitForInteractableTargets
{
	GENERATED_BODY()

	// 태스크가 시작될 때 호출됩니다. 주기적 트레이스 타이머를 켭니다.
	virtual void Activate() override;

	/**
	 * 이 태스크를 생성하는 블루프린트 노드용 정적 팩토리 함수입니다. 태스크는 자동으로 반복 실행됩니다.
	 * - InteractionQuery: 누가 상호작용을 요청하는지에 대한 정보
	 * - TraceProfile: 트레이스에 사용할 콜리전 프로파일
	 * - StartLocation: 트레이스를 시작할 위치 정보
	 * - InteractionScanRange: 트레이스 사거리, InteractionScanRate: 트레이스 주기(초)
	 * - bShowDebug: true면 트레이스 선을 화면에 디버그로 그립니다.
	 */
public:
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_WaitForInteractableTargets_SingleLineTrace* WaitForInteractableTargets_SingleLineTrace(UGameplayAbility* OwningAbility, FInteractionQuery InteractionQuery, FCollisionProfileName TraceProfile, FGameplayAbilityTargetingLocationInfo StartLocation, float InteractionScanRange = 100, float InteractionScanRate = 0.100, bool bShowDebug = false);

private:

	// 태스크가 끝날 때 호출됩니다. 타이머를 정리합니다.
	virtual void OnDestroy(bool AbilityEnded) override;

	// 타이머에 의해 주기적으로 호출되어, 라인 트레이스를 쏘고 상호작용 옵션을 갱신하는 핵심 함수입니다.
	void PerformTrace();

	UPROPERTY()
	FInteractionQuery InteractionQuery;					// 누가 요청하는지에 대한 정보

	UPROPERTY()
	FGameplayAbilityTargetingLocationInfo StartLocation;	// 트레이스를 시작할 위치

	float InteractionScanRange = 100;	// 트레이스 사거리
	float InteractionScanRate = 0.100;	// 트레이스 주기(초)
	bool bShowDebug = false;			// 디버그 선 표시 여부

	FTimerHandle TimerHandle;			// PerformTrace를 주기 호출하는 타이머 핸들
};