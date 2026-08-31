#include "EnemyMeleeTraceComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystem/Ability/Enemy/ECEnemyDamageAbility_Melee.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

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

void UEnemyMeleeTraceComponent::BeginTrace(EEnemyMeleeTracePart TracePart)
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
	
	HitActors.Reset();
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

	HitResults.Append(EndHitResults);
	
	DrawTraceDebug(PreviousTraceStart, CurrentTraceStart, ActiveTraceConfig.TraceRadius, bStartHit);
	DrawTraceDebug(PreviousTraceEnd, CurrentTraceEnd, ActiveTraceConfig.TraceRadius, bEndHit);

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