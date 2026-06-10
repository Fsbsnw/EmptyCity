#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "InteractionStatics.generated.h"

template <typename InterfaceType> class TScriptInterface;

class AActor;
class IInteractableTarget;
class UObject;
struct FFrame;
struct FHitResult;
struct FOverlapResult;

/**
 * UInteractionStatics
 *
 * 상호작용 시스템에서 자주 쓰는 보조 함수들을 모아둔 정적 함수 라이브러리(블루프린트 함수 라이브러리)입니다.
 * 주로 "충돌/트레이스 결과 → 상호작용 가능한 대상(IInteractableTarget) 목록" 변환을 담당합니다.
 */
UCLASS()
class UInteractionStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 상호작용 대상 인터페이스로부터 그것이 속한 실제 액터를 찾아 반환합니다. */
	UFUNCTION(BlueprintCallable)
	static AActor* GetActorFromInteractableTarget(TScriptInterface<IInteractableTarget> InteractableTarget);

	/** 특정 액터(또는 그 액터의 컴포넌트들) 중 상호작용 가능한 대상을 모두 찾아 OutInteractableTargets에 담습니다. */
	UFUNCTION(BlueprintCallable)
	static void GetInteractableTargetsFromActor(AActor* Actor, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets);

	/** 겹침(Overlap) 검사 결과 배열에서 상호작용 가능한 대상들을 추출해 OutInteractableTargets에 추가합니다.
	 * UAbilityTask_GrantNearbyInteraction의 구체 검사 결과를 처리할 때 사용됩니다.*/
	static void AppendInteractableTargetsFromOverlapResults(const TArray<FOverlapResult>& OverlapResults, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets);

	/** 라인 트레이스 충돌(HitResult) 결과에서 상호작용 가능한 대상들을 추출해 OutInteractableTargets에 추가합니다.
	 * UAbilityTask_WaitForInteractableTargets_SingleLineTrace의 트레이스 결과를 처리할 때 사용됩니다.*/
	static void AppendInteractableTargetsFromHitResult(const FHitResult& HitResult, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets);
};
