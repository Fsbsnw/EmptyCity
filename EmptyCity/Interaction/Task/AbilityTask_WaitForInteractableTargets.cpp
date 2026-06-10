#include "Interaction/Task/AbilityTask_WaitForInteractableTargets.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/IInteractableTarget.h"

struct FInteractionQuery;

// 시작점(Start)에서 끝점(End)으로 라인 트레이스를 수행합니다.
// 여러 충돌 결과 중 가장 가까운(첫 번째) 결과만 OutHitResult로 돌려줍니다.
void UAbilityTask_WaitForInteractableTargets::LineTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params)
{
	check(World);

	OutHitResult = FHitResult();
	TArray<FHitResult> HitResults;
	World->LineTraceMultiByProfile(HitResults, Start, End, ProfileName, Params);

	// 충돌이 없더라도 시작/끝점은 기록해 둡니다.
	OutHitResult.TraceStart = Start;
	OutHitResult.TraceEnd = End;

	// 충돌이 있으면 가장 가까운 첫 번째 결과를 사용합니다.
	if (HitResults.Num() > 0)
	{
		OutHitResult = HitResults[0];
	}
}

// 플레이어가 카메라로 보는 방향을 기준으로 실제 트레이스의 끝점(OutTraceEnd)을 계산합니다.
// 캐릭터 위치(TraceStart)와 카메라 위치가 다르기 때문에, 카메라 시점을 캐릭터 기준 조준으로 변환해 줍니다.
void UAbilityTask_WaitForInteractableTargets::AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd, bool bIgnorePitch) const
{
	if (!Ability) // 서버와 실행을 시작한 클라이언트에서만 동작합니다.
	{
		return;
	}

	//@TODO: 봇(AI)도 처리해야 할까요?
	// 카메라 시점을 얻기 위해 플레이어 컨트롤러를 가져옵니다.
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	check(PC);

	// 카메라의 위치(ViewStart)와 회전(ViewRot)을 얻습니다.
	FVector ViewStart;
	FRotator ViewRot;
	PC->GetPlayerViewPoint(ViewStart, ViewRot);

	// 카메라가 바라보는 방향으로 MaxRange만큼 떨어진 지점을 1차 끝점으로 잡습니다.
	const FVector ViewDir = ViewRot.Vector();
	FVector ViewEnd = ViewStart + (ViewDir * MaxRange);

	// 카메라 광선을 캐릭터 기준 사거리 구체 안쪽으로 잘라냅니다.
	ClipCameraRayToAbilityRange(ViewStart, ViewDir, TraceStart, MaxRange, ViewEnd);

	// 카메라 시점에서 트레이스해, 조준선이 실제로 무언가에 막히는지 확인합니다.
	FHitResult HitResult;
	LineTrace(HitResult, InSourceActor->GetWorld(), ViewStart, ViewEnd, TraceProfile.Name, Params);

	// 충돌이 있고 그 지점이 사거리 안쪽이면 충돌 지점을 조준점으로 사용합니다.
	const bool bUseTraceResult = HitResult.bBlockingHit && (FVector::DistSquared(TraceStart, HitResult.Location) <= (MaxRange * MaxRange));

	const FVector AdjustedEnd = (bUseTraceResult) ? HitResult.Location : ViewEnd;

	// 캐릭터 위치에서 조준점까지의 방향을 구합니다. 0이면 카메라 방향을 그대로 씁니다.
	FVector AdjustedAimDir = (AdjustedEnd - TraceStart).GetSafeNormal();
	if (AdjustedAimDir.IsZero())
	{
		AdjustedAimDir = ViewDir;
	}

	// 트레이스가 상하 각도에 영향을 주지 않도록 설정된 경우, 원래 카메라의 Pitch를 유지합니다.
	if (!bTraceAffectsAimPitch && bUseTraceResult)
	{
		FVector OriginalAimDir = (ViewEnd - TraceStart).GetSafeNormal();

		if (!OriginalAimDir.IsZero())
		{
			// 방향을 회전값(각도)으로 바꾼 뒤, Pitch만 원래 값으로 되돌립니다.
			const FRotator OriginalAimRot = OriginalAimDir.Rotation();

			FRotator AdjustedAimRot = AdjustedAimDir.Rotation();
			AdjustedAimRot.Pitch = OriginalAimRot.Pitch;

			AdjustedAimDir = AdjustedAimRot.Vector();
		}
	}

	// 최종 조준 방향으로 MaxRange만큼 떨어진 지점을 결과 끝점으로 돌려줍니다.
	OutTraceEnd = TraceStart + (AdjustedAimDir * MaxRange);
}

// 카메라에서 나온 광선과, 캐릭터를 중심으로 한 사거리 구체가 만나는 지점을 계산합니다.
// (구체와 광선의 교차점을 구하는 기하 계산입니다.)
// 광선이 구체와 만나면 그 진입 지점을 ClippedPosition에 담고 true를, 아니면 false를 반환합니다.
bool UAbilityTask_WaitForInteractableTargets::ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition)
{
	FVector CameraToCenter = AbilityCenter - CameraLocation;
	float DotToCenter = FVector::DotProduct(CameraToCenter, CameraDirection);
	if (DotToCenter >= 0)		// 음수이면 구체 반대편을 보고 있는 것입니다(구체 안에 있을 수도 있어 별도 처리 여지가 있음).
	{
		// 광선에서 구체 중심까지의 수직 거리(제곱)를 구합니다.
		float DistanceSquared = CameraToCenter.SizeSquared() - (DotToCenter * DotToCenter);
		float RadiusSquared = (AbilityRange * AbilityRange);
		if (DistanceSquared <= RadiusSquared)	// 그 거리가 반지름 이내이면 광선이 구체를 통과합니다.
		{
			float DistanceFromCamera = FMath::Sqrt(RadiusSquared - DistanceSquared);
			float DistanceAlongRay = DotToCenter + DistanceFromCamera;						// 더하기 대신 빼면 반대쪽 교차점이 나옵니다.
			ClippedPosition = CameraLocation + (DistanceAlongRay * CameraDirection);		// 사거리 구체에 맞춰 잘라낸 조준점
			return true;
		}
	}
	return false;
}

// 탐지된 대상들로부터 상호작용 옵션을 새로 모으고, 지금 실행 가능한 것만 추립니다.
// 그 결과가 이전 목록과 다르면 CurrentOptions를 갱신하고 변경 이벤트를 알립니다.
void UAbilityTask_WaitForInteractableTargets::UpdateInteractableOptions(const FInteractionQuery& InteractQuery, const TArray<TScriptInterface<IInteractableTarget>>& InteractableTargets)
{
	TArray<FInteractionOption> NewOptions;

	for (const TScriptInterface<IInteractableTarget>& InteractiveTarget : InteractableTargets)
	{
		// 각 대상에게 자신의 상호작용 옵션을 수집해 달라고 요청합니다.
		TArray<FInteractionOption> TempOptions;
		FInteractionOptionBuilder InteractionBuilder(InteractiveTarget, TempOptions);
		InteractiveTarget->GatherInteractionOptions(InteractQuery, InteractionBuilder);

		for (FInteractionOption& Option : TempOptions)
		{
			FGameplayAbilitySpec* InteractionAbilitySpec = nullptr;

			// 방식 2) 대상이 자체 어빌리티 시스템과 핸들을 가지고 있으면, 대상 쪽의 어빌리티를 발동합니다.
			if (Option.TargetAbilitySystem && Option.TargetInteractionAbilityHandle.IsValid())
			{
				// 핸들로 어빌리티 스펙을 찾습니다.
				InteractionAbilitySpec = Option.TargetAbilitySystem->FindAbilitySpecFromHandle(Option.TargetInteractionAbilityHandle);
			}
			// 방식 1) 부여할 어빌리티가 지정되어 있으면, 우리(플레이어) 자신에게서 그 어빌리티를 찾아 발동합니다.
			else if (Option.InteractionAbilityToGrant)
			{
				// 클래스로 어빌리티 스펙을 찾습니다.
				InteractionAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(Option.InteractionAbilityToGrant);

				if (InteractionAbilitySpec)
				{
					// 찾은 정보로 옵션을 채워, 이후 발동 시 바로 쓸 수 있게 합니다.
					Option.TargetAbilitySystem = AbilitySystemComponent.Get();
					Option.TargetInteractionAbilityHandle = InteractionAbilitySpec->Handle;
				}
			}

			if (InteractionAbilitySpec)
			{
				// 지금 당장 발동할 수 없는 옵션(쿨다운, 자원 부족 등)은 걸러냅니다.
				if (InteractionAbilitySpec->Ability->CanActivateAbility(InteractionAbilitySpec->Handle, AbilitySystemComponent->AbilityActorInfo.Get()))
				{
					NewOptions.Add(Option);
				}
			}
		}
	}

	// 새 옵션 목록이 이전 목록과 달라졌는지 검사합니다.
	bool bOptionsChanged = false;
	if (NewOptions.Num() == CurrentOptions.Num())
	{
		// 개수가 같으면, 정렬한 뒤 항목을 하나씩 비교합니다.
		NewOptions.Sort();

		for (int OptionIndex = 0; OptionIndex < NewOptions.Num(); OptionIndex++)
		{
			const FInteractionOption& NewOption = NewOptions[OptionIndex];
			const FInteractionOption& CurrentOption = CurrentOptions[OptionIndex];

			if (NewOption != CurrentOption)
			{
				bOptionsChanged = true;
				break;
			}
		}
	}
	else
	{
		// 개수가 다르면 무조건 변경된 것입니다.
		bOptionsChanged = true;
	}

	// 변경되었을 때만 목록을 갱신하고 이벤트를 알립니다(불필요한 UI 갱신 방지).
	if (bOptionsChanged)
	{
		CurrentOptions = NewOptions;
		InteractableObjectsChanged.Broadcast(CurrentOptions);
	}
}
