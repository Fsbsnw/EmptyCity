#pragma once

#include "Abilities/Tasks/AbilityTask.h"
#include "Engine/CollisionProfile.h"
#include "Interaction/InteractionOption.h"

#include "AbilityTask_WaitForInteractableTargets.generated.h"

class AActor;
class IInteractableTarget;
class UObject;
class UWorld;
struct FCollisionQueryParams;
struct FHitResult;
struct FInteractionQuery;
template <typename InterfaceType> class TScriptInterface;

// 상호작용 가능한 옵션 목록이 바뀌었을 때 외부(주로 UI)로 알리는 델리게이트입니다.
// 블루프린트에서 이 이벤트에 바인딩해 상호작용 위젯을 갱신할 수 있습니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableObjectsChangedEvent, const TArray<FInteractionOption>&, InteractableOptions);

/**
 * UAbilityTask_WaitForInteractableTargets
 *
 * "지금 상호작용할 수 있는 대상이 무엇인지"를 계속 감시하는 어빌리티 태스크들의 공통 부모 클래스입니다.
 * Abstract(추상)이라 직접 사용하지 않고, 실제로 어떻게 탐지할지는 자식 클래스가 정합니다.
 * (예: UAbilityTask_WaitForInteractableTargets_SingleLineTrace = 라인 트레이스 방식)
 *
 * 이 부모 클래스는 자식들이 공통으로 쓰는 트레이스/조준 보조 함수와,
 * 탐지 결과를 옵션 목록으로 정리해 변경 시 알려주는 로직을 제공합니다.
 */
UCLASS(Abstract)
class UAbilityTask_WaitForInteractableTargets : public UAbilityTask
{
	GENERATED_BODY()

public:
	// 상호작용 옵션 목록이 바뀔 때마다 호출되는 이벤트입니다. 보통 UI가 여기에 연결됩니다.
	UPROPERTY(BlueprintAssignable)
	FInteractableObjectsChangedEvent InteractableObjectsChanged;

protected:

	// 지정한 프로파일로 라인 트레이스를 수행하고, 가장 먼저 맞은 결과 하나를 OutHitResult에 담습니다.
	static void LineTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params);

	// 플레이어 컨트롤러의 카메라 시점을 기준으로 조준 방향을 계산해, 최종 트레이스 끝점(OutTraceEnd)을 구합니다.
	// 즉, "캐릭터 위치에서 쏘되 카메라가 보는 곳"을 향하도록 보정해 줍니다.
	void AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd, bool bIgnorePitch = false) const;

	// 카메라에서 나온 조준 광선을 어빌리티 사거리 구체 안쪽으로 잘라냅니다(clip).
	// 카메라와 캐릭터 위치가 달라 생기는 조준 오차를 사거리 내로 제한하기 위한 계산입니다.
	static bool ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition);

	// 탐지된 대상들로부터 상호작용 옵션을 모으고, 지금 실행 가능한 것만 추려
	// 이전 목록과 비교한 뒤, 달라졌으면 InteractableObjectsChanged 이벤트로 알립니다.
	void UpdateInteractableOptions(const FInteractionQuery& InteractQuery, const TArray<TScriptInterface<IInteractableTarget>>& InteractableTargets);

	FCollisionProfileName TraceProfile;	// 트레이스에 사용할 콜리전 프로파일

	// 트레이스 결과가 조준의 상하 각도(Pitch)에 영향을 주는지 여부입니다.
	bool bTraceAffectsAimPitch = true;

	TArray<FInteractionOption> CurrentOptions;	// 현재 유효한 상호작용 옵션 목록(이전 결과와 비교용)
};
