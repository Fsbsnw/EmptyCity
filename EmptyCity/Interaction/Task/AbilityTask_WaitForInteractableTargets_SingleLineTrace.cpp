#include "AbilityTask_WaitForInteractableTargets_SingleLineTrace.h"
#include "Interaction/InteractionStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

// 태스크 인스턴스를 생성하고 입력 파라미터를 채워 반환하는 팩토리 함수입니다.
UAbilityTask_WaitForInteractableTargets_SingleLineTrace* UAbilityTask_WaitForInteractableTargets_SingleLineTrace::WaitForInteractableTargets_SingleLineTrace(UGameplayAbility* OwningAbility, FInteractionQuery InteractionQuery, FCollisionProfileName TraceProfile, FGameplayAbilityTargetingLocationInfo StartLocation, float InteractionScanRange, float InteractionScanRate, bool bShowDebug)
{
	UAbilityTask_WaitForInteractableTargets_SingleLineTrace* MyObj = NewAbilityTask<UAbilityTask_WaitForInteractableTargets_SingleLineTrace>(OwningAbility);
	MyObj->InteractionScanRange = InteractionScanRange;
	MyObj->InteractionScanRate = InteractionScanRate;
	MyObj->StartLocation = StartLocation;
	MyObj->InteractionQuery = InteractionQuery;
	MyObj->TraceProfile = TraceProfile;
	MyObj->bShowDebug = bShowDebug;

	return MyObj;
}

// 태스크가 시작될 때 호출됩니다.
void UAbilityTask_WaitForInteractableTargets_SingleLineTrace::Activate()
{
	// 아바타(캐릭터)가 준비될 때까지 기다리도록 설정합니다.
	SetWaitingOnAvatar();

	// InteractionScanRate 주기로 PerformTrace를 반복 호출하는 타이머를 켭니다(마지막 인자 true = 반복).
	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::PerformTrace, InteractionScanRate, true);
}

// 태스크가 종료될 때 호출됩니다. 켜 두었던 타이머를 정리합니다.
void UAbilityTask_WaitForInteractableTargets_SingleLineTrace::OnDestroy(bool AbilityEnded)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}

	Super::OnDestroy(AbilityEnded);
}

// 타이머에 의해 주기적으로 호출됩니다.
// 플레이어가 조준한 방향으로 라인 트레이스를 쏴서, 맞은 물체의 상호작용 옵션을 갱신합니다.
void UAbilityTask_WaitForInteractableTargets_SingleLineTrace::PerformTrace()
{
	// 어빌리티의 아바타(캐릭터)를 가져옵니다. 없으면 아무것도 하지 않습니다.
	AActor* AvatarActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (!AvatarActor)
	{
		return;
	}

	UWorld* World = GetWorld();

	// 트레이스가 자기 자신(캐릭터)에 맞지 않도록 무시 목록에 추가합니다.
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AvatarActor);

	const bool bTraceComplex = false;	// 정밀(트라이앵글 단위) 트레이스는 사용하지 않습니다.
	FCollisionQueryParams Params(SCENE_QUERY_STAT(UAbilityTask_WaitForInteractableTargets_SingleLineTrace), bTraceComplex);
	Params.AddIgnoredActors(ActorsToIgnore);

	// 트레이스 시작점을 구하고, 카메라 조준 방향을 반영해 끝점(TraceEnd)을 계산합니다.
	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FVector TraceEnd;
	AimWithPlayerController(AvatarActor, Params, TraceStart, InteractionScanRange, OUT TraceEnd);

	// 시작점에서 끝점으로 라인 트레이스를 실행합니다.
	FHitResult OutHitResult;
	LineTrace(OutHitResult, World, TraceStart, TraceEnd, TraceProfile.Name, Params);

	// 맞은 결과에서 상호작용 가능한 대상을 추출합니다.
	TArray<TScriptInterface<IInteractableTarget>> InteractableTargets;
	UInteractionStatics::AppendInteractableTargetsFromHitResult(OutHitResult, InteractableTargets);

	// 추출된 대상으로 상호작용 옵션 목록을 갱신합니다(변경 시 부모 클래스가 이벤트를 알립니다).
	UpdateInteractableOptions(InteractionQuery, InteractableTargets);

#if ENABLE_DRAW_DEBUG
	// 디버그 모드일 때 트레이스 결과를 화면에 그립니다.
	// 무언가에 맞으면 빨간색, 맞지 않으면 초록색으로 표시합니다.
	if (bShowDebug)
	{
		FColor DebugColor = OutHitResult.bBlockingHit ? FColor::Red : FColor::Green;
		if (OutHitResult.bBlockingHit)
		{
			DrawDebugLine(World, TraceStart, OutHitResult.Location, DebugColor, false, InteractionScanRate);
			DrawDebugSphere(World, OutHitResult.Location, 5, 16, DebugColor, false, InteractionScanRate);
		}
		else
		{
			DrawDebugLine(World, TraceStart, TraceEnd, DebugColor, false, InteractionScanRate);
		}
	}
#endif // ENABLE_DRAW_DEBUG
}

