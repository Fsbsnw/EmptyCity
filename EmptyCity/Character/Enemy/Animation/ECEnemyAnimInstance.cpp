// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Animation/ECEnemyAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ECGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "KismetAnimationLibrary.h"

void UECEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (const APawn* OwningPawn = TryGetPawnOwner())
	{
		PreviousOwnerLocation = OwningPawn->GetActorLocation();
		bHasPreviousOwnerLocation = true;
	}
	else
	{
		bHasPreviousOwnerLocation = false;
	}

	AActor* OwningActor = GetOwningActor();
	
	if (!OwningActor)
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor);
	if (ASC)
	{
		// 설정한 태그 변화를 감지합니다.
		ASC->RegisterGameplayTagEvent(ECGameplayTags::Status_Debuff_Stun, EGameplayTagEventType::NewOrRemoved)
		   .AddUObject(this, &ThisClass::OnStunTagChanged);
		
		// 설정한 태그 변화를 감지합니다.
		ASC->RegisterGameplayTagEvent(ECGameplayTags::Status_Death_Dying, EGameplayTagEventType::NewOrRemoved)
		   .AddUObject(this, &ThisClass::OnDeathTagChanged);
	}
}

void UECEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const APawn* OwningPawn = TryGetPawnOwner();
	if (!IsValid(OwningPawn))
	{
		MoveSpeed = 0.0f;
		MoveDirection = 0.0f;
		bHasPreviousOwnerLocation = false;
		return;
	}

	const FVector CurrentOwnerLocation = OwningPawn->GetActorLocation();
	FVector EffectiveVelocity = OwningPawn->GetVelocity();

	// Sequencer의 Transform 트랙은 Pawn 위치를 직접 변경하므로 CharacterMovement의 Velocity가 0입니다.
	// 그 상태에서도 실제 프레임 간 이동량을 사용해 걷기 BlendSpace가 정상적으로 갱신되게 합니다.
	if (EffectiveVelocity.SizeSquared2D() <= KINDA_SMALL_NUMBER
		&& bHasPreviousOwnerLocation
		&& DeltaSeconds > SMALL_NUMBER)
	{
		const FVector ExternalMovementVelocity =
			(CurrentOwnerLocation - PreviousOwnerLocation) / DeltaSeconds;

		// 레벨 이동이나 순간이동은 걷기 속도로 취급하지 않습니다.
		constexpr float MaxExternalMovementSpeed = 5000.0f;
		if (ExternalMovementVelocity.SizeSquared2D() <= FMath::Square(MaxExternalMovementSpeed))
		{
			EffectiveVelocity = ExternalMovementVelocity;
		}
	}

	PreviousOwnerLocation = CurrentOwnerLocation;
	bHasPreviousOwnerLocation = true;

	MoveSpeed = EffectiveVelocity.Size2D();
	MoveDirection = UKismetAnimationLibrary::CalculateDirection(
		EffectiveVelocity,
		OwningPawn->GetActorRotation());
}

void UECEnemyAnimInstance::NativeUninitializeAnimation()
{
	bHasPreviousOwnerLocation = false;

	AActor* OwningActor = GetOwningActor();
	
	if (OwningActor)
	{
		UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor);
		if (ASC)
		{
			// 설정한 태그에 등록된 델리게이트를 제거합니다.
			ASC->RegisterGameplayTagEvent(ECGameplayTags::Status_Debuff_Stun, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
			ASC->RegisterGameplayTagEvent(ECGameplayTags::Status_Death_Dying, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
		}
	}

	Super::NativeUninitializeAnimation();
}

void UECEnemyAnimInstance::OnStunTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	// 태그 개수가 0보다 크면 스턴 진행 상태, 아니면 스턴 제거 상태
	bIsStunned = (NewCount > 0);
}

void UECEnemyAnimInstance::OnDeathTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	bIsDead = (NewCount > 0);
}
