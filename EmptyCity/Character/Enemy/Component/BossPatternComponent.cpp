#include "Character/Enemy/Component/BossPatternComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Algo/RandomShuffle.h"
#include "Character/Enemy/AI/ECEnemyAIController.h"
#include "Character/Enemy/Pattern/ECBossGroundHazard.h"
#include "Component/Attribute/ECHealthComponent.h"
#include "DrawDebugHelpers.h"
#include "ECGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffect.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UECBossPatternComponent::UECBossPatternComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	WeaponTypeTag = ECGameplayTags::Weapon_Boss;
	AttackTypeTag = ECGameplayTags::Attack_Heavy;
}

void UECBossPatternComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	HealthComponent = UECHealthComponent::FindHealthComponent(Owner);
	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.RemoveDynamic(this, &ThisClass::HandleHealthChanged);
		HealthComponent->OnHealthChanged.AddDynamic(this, &ThisClass::HandleHealthChanged);
		HealthComponent->OnDeathStarted.RemoveDynamic(this, &ThisClass::HandleOwnerDeath);
		HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::HandleOwnerDeath);
	}

	SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
}

void UECBossPatternComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.RemoveDynamic(this, &ThisClass::HandleHealthChanged);
		HealthComponent->OnDeathStarted.RemoveDynamic(this, &ThisClass::HandleOwnerDeath);
	}

	CleanupHazards();
	HealthComponent = nullptr;
	SourceASC.Reset();

	Super::EndPlay(EndPlayReason);
}

void UECBossPatternComponent::StartSequentialDetonation()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner)
		|| !Owner->HasAuthority()
		|| bDetonationStarted
		|| !bDetonationRequested)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDetonationTimerHandle);
	}

	bDetonationStarted = true;
	PreparedHazards.RemoveAll(
		[](const TObjectPtr<AECBossGroundHazard>& Hazard)
		{
			return !IsValid(Hazard) || Hazard->HasDetonated();
		});

	TArray<TObjectPtr<AECBossGroundHazard>> HazardsToArm = PreparedHazards;
	if (bRandomizeExplosionOrder)
	{
		Algo::RandomShuffle(HazardsToArm);
	}

	PendingHazardCount = HazardsToArm.Num();
	if (PendingHazardCount == 0)
	{
		CompletePattern();
		return;
	}

	UAbilitySystemComponent* AbilitySystem = SourceASC.Get();
	for (int32 Index = 0; Index < HazardsToArm.Num(); ++Index)
	{
		AECBossGroundHazard* Hazard = HazardsToArm[Index];
		if (!IsValid(Hazard))
		{
			continue;
		}

		const FGameplayEffectSpecHandle DamageSpec = MakeDamageSpec(Hazard);
		Hazard->ArmForDetonation(
			AbilitySystem,
			DamageSpec,
			FMath::Max(ExplosionInterval, 0.0f) * Index);
	}
}

void UECBossPatternComponent::ResetPattern()
{
	AActor* Owner = GetOwner();
	if (Owner && !Owner->HasAuthority())
	{
		return;
	}

	bIsResetting = true;
	CleanupHazards();
	bIsResetting = false;

	bHazardsPrepared = false;
	bDetonationRequested = false;
	bDetonationStarted = false;
	bPatternFinished = false;
	PendingHazardCount = 0;
}

void UECBossPatternComponent::SetPatternTarget(AActor* NewTarget)
{
	PatternTargetOverride = NewTarget;
}

void UECBossPatternComponent::HandleHealthChanged(
	UECHealthComponent* ChangedHealthComponent,
	const float OldValue,
	const float NewValue,
	AActor* DamageInstigator)
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority() || !IsValid(ChangedHealthComponent))
	{
		return;
	}

	const float MaxHealth = ChangedHealthComponent->GetMaxHealth();
	if (MaxHealth <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float HealthRatio = FMath::Clamp(NewValue / MaxHealth, 0.0f, 1.0f);

	if (bResetWhenHealthRestored
		&& NewValue > OldValue
		&& HealthRatio >= FMath::Clamp(ResetHealthRatio, 0.0f, 1.0f))
	{
		ResetPattern();
		return;
	}

	if (NewValue <= 0.0f || NewValue >= OldValue)
	{
		return;
	}

	if (!bHazardsPrepared
		&& HealthRatio <= FMath::Clamp(PrepareHealthRatio, 0.0f, 1.0f))
	{
		PrepareHazards();
	}

	if (!bDetonationRequested
		&& HealthRatio <= FMath::Clamp(DetonationHealthRatio, 0.0f, 1.0f))
	{
		// 큰 단일 피해로 준비 임계점을 건너뛴 경우에도 먼저 장판을 만듭니다.
		if (!bHazardsPrepared)
		{
			PrepareHazards();
		}

		RequestDetonation();
	}
}

void UECBossPatternComponent::HandleOwnerDeath(AActor* OwningActor)
{
	CleanupHazards();
}

void UECBossPatternComponent::PrepareHazards()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!IsValid(Owner) || !Owner->HasAuthority() || !World || bHazardsPrepared)
	{
		return;
	}

	bHazardsPrepared = true;

	AActor* TargetActor = ResolvePatternTarget();
	if (!HazardClass || !IsValid(TargetActor))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[%s] BossPatternComponent가 장판 클래스 또는 패턴 타겟을 찾지 못했습니다."),
			*GetNameSafe(Owner));
		OnPatternPrepared.Broadcast(this);
		return;
	}

	const int32 MinimumCount = FMath::Max(0, FMath::Min(MinHazardCount, MaxHazardCount));
	const int32 MaximumCount = FMath::Max(0, FMath::Max(MinHazardCount, MaxHazardCount));
	const int32 HazardCount = FMath::RandRange(MinimumCount, MaximumCount);

	TArray<FVector> AcceptedLocations;
	AcceptedLocations.Reserve(HazardCount);
	PreparedHazards.Reserve(HazardCount);

	for (int32 Index = 0; Index < HazardCount; ++Index)
	{
		FTransform SpawnTransform;
		if (!TryFindHazardTransform(
			TargetActor->GetActorLocation(),
			TargetActor,
			AcceptedLocations,
			SpawnTransform))
		{
			continue;
		}

		AECBossGroundHazard* Hazard = World->SpawnActorDeferred<AECBossGroundHazard>(
			HazardClass,
			SpawnTransform,
			Owner,
			Cast<APawn>(Owner),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!Hazard)
		{
			continue;
		}

		Hazard->ConfigureVisual(
			HazardRadius,
			HazardDecalRadius,
			HazardFadeInDuration,
			MaxTargetHeightDifference);
		Hazard->ConfigureKnockback(bApplyHazardKnockback, HazardKnockbackSpeed);
		Hazard->OnFinished.AddUObject(this, &ThisClass::HandleHazardFinished);
		UGameplayStatics::FinishSpawningActor(Hazard, SpawnTransform);

		PreparedHazards.Add(Hazard);
		AcceptedLocations.Add(SpawnTransform.GetLocation());
	}

	OnPatternPrepared.Broadcast(this);
}

void UECBossPatternComponent::RequestDetonation()
{
	if (bDetonationRequested)
	{
		return;
	}

	bDetonationRequested = true;
	OnDetonationRequested.Broadcast(this);

	if (!bAutoStartDetonation)
	{
		return;
	}

	if (!GetWorld() || AutoDetonationStartDelay <= KINDA_SMALL_NUMBER)
	{
		StartSequentialDetonation();
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		AutoDetonationTimerHandle,
		this,
		&ThisClass::StartSequentialDetonation,
		AutoDetonationStartDelay,
		false);
}

void UECBossPatternComponent::CompletePattern()
{
	if (bPatternFinished || bIsResetting)
	{
		return;
	}

	bPatternFinished = true;
	PendingHazardCount = 0;
	OnPatternFinished.Broadcast(this);
}

void UECBossPatternComponent::CleanupHazards()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDetonationTimerHandle);
	}

	TArray<TObjectPtr<AECBossGroundHazard>> HazardsToDestroy = MoveTemp(PreparedHazards);
	PreparedHazards.Reset();
	PendingHazardCount = 0;

	for (AECBossGroundHazard* Hazard : HazardsToDestroy)
	{
		if (!IsValid(Hazard))
		{
			continue;
		}

		Hazard->OnFinished.RemoveAll(this);
		Hazard->Destroy();
	}
}

void UECBossPatternComponent::HandleHazardFinished(AECBossGroundHazard* Hazard)
{
	if (IsValid(Hazard))
	{
		Hazard->OnFinished.RemoveAll(this);
	}

	PreparedHazards.RemoveAll(
		[Hazard](const TObjectPtr<AECBossGroundHazard>& PreparedHazard)
		{
			return !IsValid(PreparedHazard) || PreparedHazard == Hazard;
		});

	if (!bDetonationStarted || bIsResetting)
	{
		return;
	}

	PendingHazardCount = FMath::Max(0, PendingHazardCount - 1);
	if (PendingHazardCount == 0)
	{
		CompletePattern();
	}
}

AActor* UECBossPatternComponent::ResolvePatternTarget() const
{
	if (IsValid(PatternTargetOverride))
	{
		return PatternTargetOverride;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const AECEnemyAIController* AIController = OwnerPawn
		? Cast<AECEnemyAIController>(OwnerPawn->GetController())
		: nullptr;
	if (AIController && IsValid(AIController->TargetActor))
	{
		return AIController->TargetActor;
	}

	return UGameplayStatics::GetPlayerPawn(this, 0);
}

bool UECBossPatternComponent::TryFindHazardTransform(
	const FVector& Center,
	AActor* TargetActor,
	const TArray<FVector>& AcceptedLocations,
	FTransform& OutTransform) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float MinimumDistance = FMath::Max(0.0f, FMath::Min(MinSpawnDistance, MaxSpawnDistance));
	const float MaximumDistance = FMath::Max(0.0f, FMath::Max(MinSpawnDistance, MaxSpawnDistance));
	const float MinimumDistanceSquared = FMath::Square(MinimumDistance);
	const float MaximumDistanceSquared = FMath::Square(MaximumDistance);
	const float MinimumSpacingSquared = FMath::Square(FMath::Max(MinHazardSpacing, 0.0f));
	const int32 AttemptCount = FMath::Max(1, MaxLocationAttemptsPerHazard);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BossPatternGround), false);
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(TargetActor);

	for (int32 Attempt = 0; Attempt < AttemptCount; ++Attempt)
	{
		const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
		const float Distance = FMath::Sqrt(FMath::FRandRange(
			MinimumDistanceSquared,
			MaximumDistanceSquared));
		const FVector Candidate = Center + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * Distance;
		const FVector TraceStart = Candidate + FVector::UpVector * GroundTraceUpDistance;
		const FVector TraceEnd = Candidate - FVector::UpVector * GroundTraceDownDistance;

		FHitResult HitResult;
		const bool bHit = World->LineTraceSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			GroundTraceChannel,
			QueryParams);

		if (bDrawDebug)
		{
			DrawDebugLine(
				World,
				TraceStart,
				TraceEnd,
				bHit ? FColor::Green : FColor::Red,
				false,
				5.0f,
				0,
				1.5f);
		}

		if (!bHit || HitResult.ImpactNormal.Z < MinGroundNormalZ)
		{
			continue;
		}

		const FVector Location = HitResult.ImpactPoint + HitResult.ImpactNormal * SurfaceOffset;
		const bool bOverlapsAcceptedLocation = AcceptedLocations.ContainsByPredicate(
			[&Location, MinimumSpacingSquared](const FVector& AcceptedLocation)
			{
				return FVector::DistSquared2D(Location, AcceptedLocation) < MinimumSpacingSquared;
			});
		if (bOverlapsAcceptedLocation)
		{
			continue;
		}

		// Deferred Decal의 투영축이 표면 안쪽을 향하도록 -Normal을 사용합니다.
		const FRotator Rotation = (-HitResult.ImpactNormal).Rotation();
		OutTransform = FTransform(Rotation, Location);

		if (bDrawDebug)
		{
			// 빨강은 실제 피격 반경, 청록은 데칼 투영 반경입니다.
			DrawDebugSphere(World, Location, HazardRadius, 24, FColor::Red, false, 5.0f, 0, 2.0f);
			DrawDebugSphere(World, Location, HazardDecalRadius, 24, FColor::Cyan, false, 5.0f, 0, 2.0f);
		}

		return true;
	}

	return false;
}

FGameplayEffectSpecHandle UECBossPatternComponent::MakeDamageSpec(
	AECBossGroundHazard* Hazard) const
{
	UAbilitySystemComponent* AbilitySystem = SourceASC.Get();
	AActor* Owner = GetOwner();
	if (!AbilitySystem || !DamageEffectClass || !IsValid(Owner) || !IsValid(Hazard))
	{
		return FGameplayEffectSpecHandle();
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystem->MakeEffectContext();
	EffectContext.AddInstigator(Owner, Owner);
	EffectContext.AddSourceObject(Hazard);
	EffectContext.AddOrigin(Hazard->GetActorLocation());

	FGameplayEffectSpecHandle DamageSpec = AbilitySystem->MakeOutgoingSpec(
		DamageEffectClass,
		FMath::Max(DamageEffectLevel, 0.0f),
		EffectContext);
	if (!DamageSpec.IsValid())
	{
		return DamageSpec;
	}

	if (WeaponTypeTag.IsValid())
	{
		DamageSpec.Data->AddDynamicAssetTag(WeaponTypeTag);
	}
	if (AttackTypeTag.IsValid())
	{
		DamageSpec.Data->AddDynamicAssetTag(AttackTypeTag);
	}
	DamageSpec.Data->AddDynamicAssetTag(ECGameplayTags::Character_Type_Enemy);
	DamageSpec.Data->SetSetByCallerMagnitude(
		ECGameplayTags::SetByCaller_DamageMultiplier,
		DamageMultiplier);
	DamageSpec.Data->SetSetByCallerMagnitude(
		ECGameplayTags::SetByCaller_StaminaDamage,
		StaminaDamage);

	return DamageSpec;
}
