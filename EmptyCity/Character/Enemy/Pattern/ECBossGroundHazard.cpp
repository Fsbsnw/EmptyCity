#include "Character/Enemy/Pattern/ECBossGroundHazard.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/DecalComponent.h"
#include "Components/SceneComponent.h"
#include "ECGameplayTags.h"
#include "Engine/OverlapResult.h"
#include "GenericTeamAgentInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

AECBossGroundHazard::AECBossGroundHazard()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	WarningDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("WarningDecal"));
	WarningDecal->SetupAttachment(SceneRoot);
}

void AECBossGroundHazard::BeginPlay()
{
	Super::BeginPlay();

	ApplyVisualConfig();
	StartFadeIn();
}

void AECBossGroundHazard::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DetonationTimerHandle);
	}

	if (HasAuthority())
	{
		BroadcastFinishedOnce();
	}

	Super::EndPlay(EndPlayReason);
}

void AECBossGroundHazard::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DamageRadius);
	DOREPLIFETIME(ThisClass, DecalRadius);
	DOREPLIFETIME(ThisClass, FadeInDuration);
	DOREPLIFETIME(ThisClass, MaxTargetHeightDifference);
	DOREPLIFETIME(ThisClass, bHasDetonated);
}

void AECBossGroundHazard::ConfigureVisual(
	const float InDamageRadius,
	const float InDecalRadius,
	const float InFadeInDuration,
	const float InMaxTargetHeightDifference)
{
	DamageRadius = FMath::Max(InDamageRadius, 0.0f);
	DecalRadius = FMath::Max(InDecalRadius, 0.0f);
	FadeInDuration = FMath::Max(InFadeInDuration, 0.0f);
	MaxTargetHeightDifference = FMath::Max(InMaxTargetHeightDifference, 0.0f);

	ApplyVisualConfig();
}

void AECBossGroundHazard::ConfigureKnockback(
	const bool bInApplyKnockback,
	const float InKnockbackSpeed)
{
	bApplyKnockback = bInApplyKnockback;
	KnockbackSpeed = FMath::Max(InKnockbackSpeed, 0.0f);
}

void AECBossGroundHazard::ArmForDetonation(
	UAbilitySystemComponent* InSourceASC,
	const FGameplayEffectSpecHandle& InDamageSpec,
	const float Delay)
{
	if (!HasAuthority() || bHasDetonated)
	{
		return;
	}

	SourceASC = InSourceASC;
	DamageSpec = InDamageSpec;

	if (!GetWorld() || Delay <= KINDA_SMALL_NUMBER)
	{
		Detonate();
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		DetonationTimerHandle,
		this,
		&ThisClass::Detonate,
		Delay,
		false);
}

void AECBossGroundHazard::Detonate()
{
	if (!HasAuthority() || bHasDetonated)
	{
		return;
	}

	bHasDetonated = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DetonationTimerHandle);
	}

	ApplyDamageToOverlappingTargets();
	MulticastPlayExplosion();
	BroadcastFinishedOnce();

	if (PostExplosionLifeTime <= KINDA_SMALL_NUMBER)
	{
		Destroy();
	}
	else
	{
		SetLifeSpan(PostExplosionLifeTime);
	}
}

void AECBossGroundHazard::OnRep_VisualConfig()
{
	ApplyVisualConfig();

	if (HasActorBegunPlay() && !bHasDetonated)
	{
		StartFadeIn();
	}
}

void AECBossGroundHazard::OnRep_Detonated()
{
	if (bHasDetonated && WarningDecal)
	{
		WarningDecal->SetVisibility(false);
	}
}

void AECBossGroundHazard::MulticastPlayExplosion_Implementation()
{
	bHasDetonated = true;

	if (WarningDecal)
	{
		WarningDecal->SetVisibility(false);
	}

	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (ExplosionVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			ExplosionVFX,
			GetActorLocation(),
			GetActorRotation());
	}

	if (ExplosionSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSFX, GetActorLocation());
	}
}

void AECBossGroundHazard::ApplyVisualConfig()
{
	if (!WarningDecal)
	{
		return;
	}

	// Deferred Decal의 Y/Z가 원형 장판의 반지름이며 X는 투영 깊이입니다.
	WarningDecal->DecalSize = FVector(
		FMath::Max(DecalProjectionDepth, 0.1f),
		DecalRadius,
		DecalRadius);

	if (WarningDecalMaterial)
	{
		WarningDecal->SetDecalMaterial(WarningDecalMaterial);
	}
}

void AECBossGroundHazard::StartFadeIn()
{
	if (!WarningDecal || GetNetMode() == NM_DedicatedServer || bHasDetonated)
	{
		return;
	}

	UMaterialInterface* Material = WarningDecalMaterial
		? WarningDecalMaterial.Get()
		: WarningDecal->GetDecalMaterial();
	if (!Material)
	{
		return;
	}

	WarningDecal->SetDecalMaterial(Material);
	WarningDecal->SetVisibility(true);
	WarningDecal->SetFadeIn(0.0f, FMath::Max(FadeInDuration, 0.0f));
}

void AECBossGroundHazard::ApplyDamageToOverlappingTargets()
{
	UWorld* World = GetWorld();
	UAbilitySystemComponent* SourceAbilitySystem = SourceASC.Get();
	if (!World || !SourceAbilitySystem || !DamageSpec.IsValid() || DamageRadius <= 0.0f)
	{
		return;
	}

	const float QueryRadius = FMath::Sqrt(
		FMath::Square(DamageRadius) + FMath::Square(MaxTargetHeightDifference));

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BossGroundHazard), false);
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());

	TArray<FOverlapResult> OverlapResults;
	const bool bHasOverlap = World->OverlapMultiByObjectType(
		OverlapResults,
		GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(QueryRadius),
		QueryParams);

	if (!bHasOverlap)
	{
		return;
	}

	TSet<TWeakObjectPtr<AActor>> DamagedActors;
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* TargetActor = OverlapResult.GetActor();
		if (!IsValid(TargetActor) || DamagedActors.Contains(TargetActor))
		{
			continue;
		}

		const FVector ToTarget = TargetActor->GetActorLocation() - GetActorLocation();
		if (FMath::Square(ToTarget.X) + FMath::Square(ToTarget.Y) > FMath::Square(DamageRadius)
			|| FMath::Abs(ToTarget.Z) > MaxTargetHeightDifference
			|| !CanDamageTarget(TargetActor))
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC =
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
		if (!TargetASC)
		{
			continue;
		}

		DamagedActors.Add(TargetActor);
		SourceAbilitySystem->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
		SendKnockbackEvent(TargetActor);
	}
}

void AECBossGroundHazard::SendKnockbackEvent(AActor* TargetActor) const
{
	if (!bApplyKnockback || KnockbackSpeed <= 0.0f || !IsValid(TargetActor))
	{
		return;
	}

	UAbilitySystemComponent* SourceAbilitySystem = SourceASC.Get();
	AActor* SourceActor = SourceAbilitySystem ? SourceAbilitySystem->GetAvatarActor() : nullptr;
	if (!IsValid(SourceActor))
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = ECGameplayTags::GameplayEvent_HitReaction_Knockback;
	EventData.Instigator = SourceActor;
	EventData.Target = TargetActor;
	EventData.OptionalObject = this;
	EventData.EventMagnitude = KnockbackSpeed;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		TargetActor,
		EventData.EventTag,
		EventData);
}

bool AECBossGroundHazard::CanDamageTarget(AActor* TargetActor) const
{
	UAbilitySystemComponent* SourceAbilitySystem = SourceASC.Get();
	AActor* SourceActor = SourceAbilitySystem ? SourceAbilitySystem->GetAvatarActor() : nullptr;
	if (!IsValid(TargetActor) || !IsValid(SourceActor) || TargetActor == SourceActor)
	{
		return false;
	}

	const APawn* SourcePawn = Cast<APawn>(SourceActor);
	const APawn* TargetPawn = Cast<APawn>(TargetActor);
	const AController* SourceController = SourcePawn ? SourcePawn->GetController() : nullptr;
	const AController* TargetController = TargetPawn ? TargetPawn->GetController() : nullptr;

	return !SourceController
		|| !TargetController
		|| FGenericTeamId::GetAttitude(SourceController, TargetController) != ETeamAttitude::Friendly;
}

void AECBossGroundHazard::BroadcastFinishedOnce()
{
	if (bFinishedBroadcast)
	{
		return;
	}

	bFinishedBroadcast = true;
	OnFinished.Broadcast(this);
}
