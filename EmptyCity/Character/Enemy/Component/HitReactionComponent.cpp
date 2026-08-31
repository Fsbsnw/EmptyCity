#include "HitReactionComponent.h"

#include "ECGameplayTags.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "AbilitySystem/Attribute/ECHealthSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UHitReactionComponent::UHitReactionComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = true;
	
	ASC = nullptr;
}

void UHitReactionComponent::InitializeWithAbilitySystem(UECAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);
	
	ASC = InASC;
}

void UHitReactionComponent::PlayHitStop(float TimeDuration, float TimeDilation)
{
	if (!bCanPlayHitStop)
	{
		return;
	}
    
	// 1. 배속을 적용하여 현재 액터의 시간을 느리게 만듦
	GetOwner()->CustomTimeDilation = TimeDilation;

	// 2. 다단 히트 방지 (기존 타이머 취소)
	GetWorld()->GetTimerManager().ClearTimer(HitStopTimerHandle);

	// 3. 원상복구 타이머 실행
	GetWorld()->GetTimerManager().SetTimer(HitStopTimerHandle, this, &ThisClass::RestoreTimeDilation, TimeDuration, false);
}

void UHitReactionComponent::RestoreTimeDilation()
{
	GetOwner()->CustomTimeDilation = 1.f;
}

void UHitReactionComponent::PlayWeakHitReaction(const FVector& HitSourceLocation)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}
	
	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();

	if (!AnimInstance || !HitReactionMontage || ASC->HasMatchingGameplayTag(ECGameplayTags::Status_Death_Dying))
	{
		return;
	}

	AnimInstance->Montage_Play(HitReactionMontage,1.0f, EMontagePlayReturnType::MontageLength,0.0f,false);
}

void UHitReactionComponent::ApplyGroundKnockback(const FVector& Direction, float InitialSpeed, float Duration)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement())
	{
		return;
	}

	KnockbackDirection = Direction.GetSafeNormal2D();
	KnockbackInitialSpeed = FMath::Max(InitialSpeed, 0.0f);
	KnockbackDuration = FMath::Max(Duration, 0.01f);
	KnockbackElapsedTime = 0.0f;
	bGroundKnockbackActive = !KnockbackDirection.IsNearlyZero() && KnockbackInitialSpeed > 0.0f;

	SetComponentTickEnabled(bGroundKnockbackActive);
}

void UHitReactionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bGroundKnockbackActive)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	UCharacterMovementComponent* Movement = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
	if (!Movement)
	{
		StopGroundKnockback();
		return;
	}

	KnockbackElapsedTime += DeltaTime;
	const float Alpha = FMath::Clamp(KnockbackElapsedTime / KnockbackDuration, 0.0f, 1.0f);
	const float Speed = KnockbackInitialSpeed * FMath::Square(1.0f - Alpha);

	FVector KnockbackVelocity = KnockbackDirection * Speed;
	KnockbackVelocity.Z = Movement->Velocity.Z;
	Movement->Velocity = KnockbackVelocity;

	if (Alpha >= 1.0f)
	{
		StopGroundKnockback();
	}
}

void UHitReactionComponent::StopGroundKnockback()
{
	bGroundKnockbackActive = false;
	KnockbackElapsedTime = 0.0f;
	SetComponentTickEnabled(false);
}