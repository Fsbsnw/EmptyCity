#include "WeaponTraceComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/MeshComponent.h"
#include "DrawDebugHelpers.h"
#include "ECGameplayTags.h"
#include "ECWeaponInstance.h"
#include "Data/Weapon/ECWeaponSettings.h"
#include "Data/Weapon/ECWeaponTableRow.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace
{
	const FName MeleeTraceProfileName(TEXT("EC_MeleeTrace"));
}

UWeaponTraceComponent::UWeaponTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// 캐릭터 애니메이션과 무기 부착 위치가 갱신된 후 판정합니다.
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UWeaponTraceComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
	
	TraceMesh = GetOwner()->FindComponentByClass<UMeshComponent>();
}

void UWeaponTraceComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bTraceActive)
	{
		return;
	}

	UpdateTrace();
}

const FECWeaponAttackDefinition* UWeaponTraceComponent::SetActiveAttack(const FGameplayTag& AttackTag)
{
	EndTrace();
	ActiveAttackTag = FGameplayTag();

	const FECWeaponAttackDefinition* AttackDefinition = FindAttackDefinition(AttackTag);
	if (AttackDefinition)
	{
		ActiveAttackTag = AttackDefinition->AttackTag;
	}

	return AttackDefinition;
}

const FECWeaponAttackDefinition* UWeaponTraceComponent::FindAttackDefinition(const FGameplayTag& AttackTag) const
{
	if (!AttackTag.IsValid())
	{
		return nullptr;
	}

	return AttackDefinitions.FindByPredicate(
		[&AttackTag](const FECWeaponAttackDefinition& AttackDefinition)
		{
			return AttackDefinition.AttackTag.MatchesTagExact(AttackTag);
		});
}

void UWeaponTraceComponent::ClearActiveAttack()
{
	EndTrace();
	ActiveAttackTag = FGameplayTag();
}

void UWeaponTraceComponent::BeginTrace()
{
	if (bTraceActive)
	{
		return;
	}

	const FECWeaponAttackDefinition* AttackDefinition = GetActiveAttackDefinition();
	if (!IsValid(GetOwner()) || !AttackDefinition)
	{
		return;
	}
	
	HitActors.Reset();
	bHasPreviousTracePositions = false;
	bTraceActive = true;
	SetComponentTickEnabled(true);
}

void UWeaponTraceComponent::EndTrace()
{
	bTraceActive = false;
	SetComponentTickEnabled(false);
	HitActors.Reset();
	PreviousTraceStart = FVector::ZeroVector;
	PreviousTraceEnd = FVector::ZeroVector;
	bHasPreviousTracePositions = false;
}

void UWeaponTraceComponent::UpdateTrace()
{
	const FECWeaponAttackDefinition* AttackDefinition = GetActiveAttackDefinition();
	UMeshComponent* MeshComponent = TraceMesh.Get();
	if (!AttackDefinition || !IsValid(MeshComponent) || !IsValid(GetWorld()))
	{
		EndTrace();
		return;
	}

	const FECWeaponTraceConfig& TraceConfig = AttackDefinition->TraceConfig;
	if (!MeshComponent->DoesSocketExist(TraceConfig.TraceStartSocket) || !MeshComponent->DoesSocketExist(TraceConfig.TraceEndSocket))
	{
		EndTrace();
		return;
	}

	const FVector CurrentTraceStart = MeshComponent->GetSocketLocation(TraceConfig.TraceStartSocket);
	const FVector CurrentTraceEnd = MeshComponent->GetSocketLocation(TraceConfig.TraceEndSocket);
	const float TraceRadius = FMath::Max(TraceConfig.TraceRadius, 0.1f);

	// 현재 무기 길이 전체를 검사합니다.
	PerformSweep(CurrentTraceStart, CurrentTraceEnd, TraceRadius);

	// 이전 프레임과 현재 프레임 사이의 무기 궤적을 검사합니다.
	if (bHasPreviousTracePositions)
	{
		const int32 SampleCount = FMath::Max(TraceConfig.SampleCount, 2);
		for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
		{
			const float Alpha = static_cast<float>(SampleIndex) / static_cast<float>(SampleCount - 1);
			const FVector PreviousPoint = FMath::Lerp(PreviousTraceStart, PreviousTraceEnd, Alpha);
			const FVector CurrentPoint = FMath::Lerp(CurrentTraceStart, CurrentTraceEnd, Alpha);

			PerformSweep(PreviousPoint, CurrentPoint, TraceRadius);
		}
	}

	PreviousTraceStart = CurrentTraceStart;
	PreviousTraceEnd = CurrentTraceEnd;
	bHasPreviousTracePositions = true;
}

const FECWeaponAttackDefinition* UWeaponTraceComponent::GetActiveAttackDefinition() const
{
	return FindAttackDefinition(ActiveAttackTag);
}

void UWeaponTraceComponent::PerformSweep(const FVector& Start, const FVector& End, float Radius)
{
	UWorld* World = GetWorld();
	AActor* WeaponActor = GetOwner();
	if (!IsValid(World) || !IsValid(WeaponActor))
	{
		return;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WeaponTrace), false);
	QueryParams.AddIgnoredActor(WeaponActor);

	AActor* AttackOwner = WeaponActor->GetOwner();
	if (IsValid(AttackOwner))
	{
		QueryParams.AddIgnoredActor(AttackOwner);
	}

	TArray<FHitResult> HitResults;
	World->SweepMultiByProfile(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		MeleeTraceProfileName,
		FCollisionShape::MakeSphere(Radius),
		QueryParams);

	if (bDrawDebugTrace)
	{
		DrawDebugSweep(Start, End, Radius, HitResults);
	}

	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!IsValid(HitActor) || HitActor == WeaponActor || HitActor == AttackOwner || HitResult.bBlockingHit)
		{
			continue;
		}

		const TWeakObjectPtr<AActor> HitActorPtr(HitActor);
		if (HitActors.Contains(HitActorPtr))
		{
			continue;
		}

		HitActors.Add(HitActorPtr);
		ApplyDamageToHitTarget(HitResult);
		
		if (bDrawDebugTrace)
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] 무기 공격 판정: %s"), *GetNameSafe(WeaponActor), *GetNameSafe(HitActor));
		}
	}
}

void UWeaponTraceComponent::DrawDebugSweep(const FVector& Start, const FVector& End, float Radius, const TArray<FHitResult>& HitResults) const
{
#if ENABLE_DRAW_DEBUG
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	const FColor TraceColor = HitResults.IsEmpty() ? FColor::Green : FColor::Red;
	const FVector TraceVector = End - Start;
	if (TraceVector.IsNearlyZero())
	{
		DrawDebugSphere(World, Start, Radius, 12, TraceColor, false, 5.0f);
	}
	else
	{
		DrawDebugCapsule(
			World,
			(Start + End) * 0.5f,
			TraceVector.Size() * 0.5f + Radius,
			Radius,
			FRotationMatrix::MakeFromZ(TraceVector).ToQuat(),
			TraceColor,
			false,
			5.0f);
	}

	for (const FHitResult& HitResult : HitResults)
	{
		DrawDebugPoint(World, HitResult.ImpactPoint, 8.0f, FColor::Yellow, false, 5.0f);
	}
#endif
}

void UWeaponTraceComponent::SetOwningWeaponInstance(UECWeaponInstance* InWeaponInstance)
{	
	OwningWeaponInstance = InWeaponInstance;
}

void UWeaponTraceComponent::ApplyDamageToHitTarget(const FHitResult& HitResult)
{
	const FECWeaponAttackDefinition* AttackDefinition =
		GetActiveAttackDefinition();

	UECWeaponInstance* WeaponInstance =
		OwningWeaponInstance.Get();

	const FECWeaponTableRow* WeaponData = IsValid(WeaponInstance) ? WeaponInstance->GetWeaponData() : nullptr;

	const UECWeaponSettings* WeaponSettings = GetDefault<UECWeaponSettings>();

	AActor* WeaponActor = GetOwner();
	AActor* AttackOwner =IsValid(WeaponActor) ? WeaponActor->GetOwner() : nullptr;

	AActor* TargetActor = HitResult.GetActor();

	if (!AttackDefinition ||
		!AttackDefinition->GameplayEffectClass ||
		!WeaponData ||
		!WeaponSettings ||
		!IsValid(AttackOwner) ||
		!IsValid(TargetActor))
	{
		return;
	}

	// 데미지는 서버에서만 적용합니다.
	if (!AttackOwner->HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AttackOwner);
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);

	if (!SourceASC || !TargetASC)
	{
		return;
	}

	const bool bHeavyAttack = ActiveAttackTag.MatchesTagExact(ECGameplayTags::Ability_Type_Action_Attack_Heavy);

	const float DamageMultiplier = bHeavyAttack ? WeaponSettings->HeavyAttackDamageMultiplier : WeaponSettings->LightAttackDamageMultiplier;

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddInstigator(AttackOwner, WeaponActor);
	EffectContext.AddSourceObject(WeaponActor);
	EffectContext.AddHitResult(HitResult);

	FGameplayEffectSpecHandle DamageSpec = SourceASC->MakeOutgoingSpec( AttackDefinition->GameplayEffectClass, 1.0f, EffectContext);

	if (!DamageSpec.IsValid())
	{
		return;
	}

	DamageSpec.Data->AddDynamicAssetTag(ECGameplayTags::Character_Type_Player);

	DamageSpec.Data->SetSetByCallerMagnitude(ECGameplayTags::SetByCaller_WeaponAttackPower, WeaponData->AttackPower);

	DamageSpec.Data->SetSetByCallerMagnitude(ECGameplayTags::SetByCaller_DamageMultiplier, DamageMultiplier);

	SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
}
