#include "EnemyMeleeTraceComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystem/Ability/Enemy/ECEnemyDamageAbility_Melee.h"
#include "Components/DecalComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"

UEnemyMeleeTraceComponent::UEnemyMeleeTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UEnemyMeleeTraceComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		OwnerMesh = OwnerCharacter->GetMesh();
	}
}

void UEnemyMeleeTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bAttackTracing)
	{
		PerformTrace();
	}
}

void UEnemyMeleeTraceComponent::BeginTrace(
	EEnemyMeleeTracePart TracePart,
	const FEnemyWeaponGroundDecalConfig& WeaponGroundDecalConfig)
{
	if (!OwnerMesh)
	{
		return;
	}

	const FEnemyMeleeTraceConfig* FoundConfig = TracePartConfigs.Find(TracePart);
	if (!FoundConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 근접 트레이스 설정을 찾지 못했습니다."), *GetNameSafe(GetOwner()));
		return;
	}

	if (!OwnerMesh->DoesSocketExist(FoundConfig->StartSocket) ||
		!OwnerMesh->DoesSocketExist(FoundConfig->EndSocket))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 근접 트레이스 소켓이 존재하지 않습니다."), *GetNameSafe(GetOwner()));
		return;
	}

	ActiveTraceConfig = *FoundConfig;
	ActiveTracePart = TracePart;
	ActiveWeaponGroundDecalConfig = TracePart == EEnemyMeleeTracePart::Weapon
		? WeaponGroundDecalConfig
		: FEnemyWeaponGroundDecalConfig();
	
	HitActors.Reset();
	bWeaponGroundDecalSpawned = false;
	PreviousTraceStart = OwnerMesh->GetSocketLocation(ActiveTraceConfig.StartSocket);
	PreviousTraceEnd = OwnerMesh->GetSocketLocation(ActiveTraceConfig.EndSocket);

	bAttackTracing = true;
	SetComponentTickEnabled(true);
}

void UEnemyMeleeTraceComponent::EndTrace()
{
	SetComponentTickEnabled(false);
	bAttackTracing = false;
	HitActors.Reset();
	ActiveWeaponGroundDecalConfig = FEnemyWeaponGroundDecalConfig();
	bWeaponGroundDecalSpawned = false;
}

void UEnemyMeleeTraceComponent::PerformTrace()
{
	if (!OwnerMesh)
	{
		return;
	}
	
	const FVector CurrentTraceStart = OwnerMesh->GetSocketLocation(ActiveTraceConfig.StartSocket);
	const FVector CurrentTraceEnd = OwnerMesh->GetSocketLocation(ActiveTraceConfig.EndSocket);

	TArray<FHitResult> HitResults;
	TArray<FHitResult> EndHitResults;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	const bool bStartHit = GetWorld()->SweepMultiByProfile(
		HitResults,
		PreviousTraceStart,
		CurrentTraceStart,
		FQuat::Identity,
		FName(TEXT("EC_MeleeTrace")),
		FCollisionShape::MakeSphere(ActiveTraceConfig.TraceRadius),
		QueryParams);

	const bool bEndHit = GetWorld()->SweepMultiByProfile(
		EndHitResults,
		PreviousTraceEnd,
		CurrentTraceEnd,
		FQuat::Identity,
		FName(TEXT("EC_MeleeTrace")),
		FCollisionShape::MakeSphere(ActiveTraceConfig.TraceRadius),
		QueryParams);

	DrawTraceDebug(PreviousTraceStart, CurrentTraceStart, ActiveTraceConfig.TraceRadius, bStartHit);
	DrawTraceDebug(PreviousTraceEnd, CurrentTraceEnd, ActiveTraceConfig.TraceRadius, bEndHit);

	if (ActiveTracePart == EEnemyMeleeTracePart::Weapon
		&& ActiveWeaponGroundDecalConfig.bEnabled)
	{
		const FVector WeaponDirection = CurrentTraceEnd - CurrentTraceStart;

		// 무기 끝 소켓의 충돌을 우선 사용해야 타격 지점과 휘두른 방향이 가장 정확합니다.
		for (const FHitResult& Hit : EndHitResults)
		{
			if (Hit.bBlockingHit)
			{
				TrySpawnWeaponGroundDecal(
					Hit,
					CurrentTraceEnd - PreviousTraceEnd,
					WeaponDirection);

				if (bWeaponGroundDecalSpawned)
				{
					break;
				}
			}
		}

		for (const FHitResult& Hit : HitResults)
		{
			if (bWeaponGroundDecalSpawned)
			{
				break;
			}

			if (Hit.bBlockingHit)
			{
				TrySpawnWeaponGroundDecal(
					Hit,
					CurrentTraceStart - PreviousTraceStart,
					WeaponDirection);
			}
		}
	}

	HitResults.Append(EndHitResults);

	// 히트된 플레이어가 있으면 해당 정보를 GA에 전달 후 데미지 처리
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();

		if (!HitActor || !HitActor->IsA<APawn>() || HitActors.Contains(HitActor))
		{
			continue;
		}

		HitActors.Add(HitActor);

		FGameplayEventData EventData;
		EventData.EventTag = ECGameplayTags::GameplayEvent_Attack_Melee_Hit;
		EventData.Instigator = GetOwner();
		EventData.Target = HitActor;
		EventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(),ECGameplayTags::GameplayEvent_Attack_Melee_Hit,EventData);
	}

	PreviousTraceStart = CurrentTraceStart;
	PreviousTraceEnd = CurrentTraceEnd;
}

void UEnemyMeleeTraceComponent::TrySpawnWeaponGroundDecal(
	const FHitResult& HitResult,
	const FVector& SwingDirection,
	const FVector& WeaponDirection)
{
	if (bWeaponGroundDecalSpawned
		|| !ActiveWeaponGroundDecalConfig.bEnabled
		|| !IsValid(ActiveWeaponGroundDecalConfig.DecalMaterial))
	{
		return;
	}

	const FVector SurfaceNormal = HitResult.ImpactNormal.GetSafeNormal();
	if (SurfaceNormal.IsNearlyZero()
		|| SurfaceNormal.Z < ActiveWeaponGroundDecalConfig.MinimumGroundNormalZ)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World) || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	FVector DecalDirection = FVector::VectorPlaneProject(SwingDirection, SurfaceNormal);
	if (DecalDirection.IsNearlyZero())
	{
		DecalDirection = FVector::VectorPlaneProject(WeaponDirection, SurfaceNormal);
	}

	if (DecalDirection.IsNearlyZero() && IsValid(GetOwner()))
	{
		DecalDirection = FVector::VectorPlaneProject(
			GetOwner()->GetActorForwardVector(),
			SurfaceNormal);
	}

	DecalDirection.Normalize();
	if (DecalDirection.IsNearlyZero())
	{
		return;
	}

	// Deferred Decal의 X축은 투영 방향이므로 지면 안쪽(-Normal)을 향하게 합니다.
	// 텍스처의 V(세로)는 무기가 이동한 방향인 로컬 Y축에 맞춥니다.
	const FQuat SurfaceOrientation = FRotationMatrix::MakeFromXY(
		-SurfaceNormal,
		DecalDirection).ToQuat();
	const FRotator SurfaceRotation =
		(SurfaceOrientation * ActiveWeaponGroundDecalConfig.RotationOffset.Quaternion()).Rotator();

	UDecalComponent* SpawnedDecal = nullptr;
	if (ActiveWeaponGroundDecalConfig.bAttachToHitComponent && HitResult.GetComponent())
	{
		SpawnedDecal = UGameplayStatics::SpawnDecalAttached(
			ActiveWeaponGroundDecalConfig.DecalMaterial,
			ActiveWeaponGroundDecalConfig.DecalSize,
			HitResult.GetComponent(),
			HitResult.BoneName,
			HitResult.ImpactPoint,
			SurfaceRotation,
			EAttachLocation::KeepWorldPosition,
			ActiveWeaponGroundDecalConfig.LifeSpan);
	}
	else
	{
		SpawnedDecal = UGameplayStatics::SpawnDecalAtLocation(
			World,
			ActiveWeaponGroundDecalConfig.DecalMaterial,
			ActiveWeaponGroundDecalConfig.DecalSize,
			HitResult.ImpactPoint,
			SurfaceRotation,
			ActiveWeaponGroundDecalConfig.LifeSpan);
	}

	if (!IsValid(SpawnedDecal))
	{
		return;
	}

	bWeaponGroundDecalSpawned = true;

	if (bDrawDebug)
	{
		const FVector ArrowStart = HitResult.ImpactPoint + SurfaceNormal * 3.0f;
		DrawDebugDirectionalArrow(
			World,
			ArrowStart,
			ArrowStart + DecalDirection * 100.0f,
			20.0f,
			FColor::Cyan,
			false,
			1.0f,
			0,
			2.0f);
	}

	if (ActiveWeaponGroundDecalConfig.LifeSpan > 0.0f
		&& ActiveWeaponGroundDecalConfig.FadeOutDuration > 0.0f)
	{
		const float FadeDuration = FMath::Min(
			ActiveWeaponGroundDecalConfig.FadeOutDuration,
			ActiveWeaponGroundDecalConfig.LifeSpan);
		SpawnedDecal->SetFadeOut(
			ActiveWeaponGroundDecalConfig.LifeSpan - FadeDuration,
			FadeDuration,
			false);
	}
}

void UEnemyMeleeTraceComponent::DrawTraceDebug(const FVector& Start, const FVector& End, float Radius, bool bHit) const
{
	if (!bDrawDebug)
	{
		return;
	}

	const FVector Center = (Start + End) * 0.5f;
	const FVector Direction = End - Start;
	const float HalfHeight = Direction.Size() * 0.5f + Radius;
	const FQuat Rotation = FRotationMatrix::MakeFromZ(Direction).ToQuat();

	DrawDebugCapsule(
		GetWorld(),
		Center,
		HalfHeight,
		Radius,
		Rotation,
		bHit ? FColor::Green : FColor::Red,
		false,
		DebugLifetime);
}
