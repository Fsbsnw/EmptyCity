// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/Enemy/ECEnemyDamageAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ECGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GenericTeamAgentInterface.h"

UECEnemyDamageAbility::UECEnemyDamageAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UECEnemyDamageAbility::ApplyDamageToTarget(AActor* TargetActor, const FHitResult* HitResult)
{
	if (!TargetActor || !DamageEffectClass)
	{
		return false;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);

	AActor* Attacker = GetAvatarActorFromActorInfo();
	
	if (!SourceASC || !TargetASC || !IsValid(Attacker))
	{
		return false;
	}

	const APawn* SourcePawn = Cast<APawn>(Attacker);
	const APawn* TargetPawn = Cast<APawn>(TargetActor);

	const AController* SourceController = SourcePawn ? SourcePawn->GetController() : nullptr;
	const AController* TargetController = TargetPawn ? TargetPawn->GetController() : nullptr;

	if (SourceController && TargetController)
	{
		const ETeamAttitude::Type Attitude = FGenericTeamId::GetAttitude(SourceController, TargetController);

		if (Attitude == ETeamAttitude::Friendly)
		{
			return false;
		}
	}

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	
	// 적 자신이 공격자이자 직접 공격 수단입니다.
	EffectContext.AddInstigator(Attacker, Attacker);
	EffectContext.AddSourceObject(this);

	if (HitResult)
	{
		EffectContext.AddHitResult(*HitResult, true);
	}

	FGameplayEffectSpecHandle DamageSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass,	GetAbilityLevel(),EffectContext);

	// Combat GameplayCue에 전달할 태그를 저장합니다.
	DamageSpecHandle.Data->AddDynamicAssetTag(WeaponTypeTag);
	DamageSpecHandle.Data->AddDynamicAssetTag(AttackTypeTag);
	DamageSpecHandle.Data->AddDynamicAssetTag(ECGameplayTags::Character_Type_Enemy);

	// 체력 데미지와 스태미나 데미지 값을 세팅합니다.
	DamageSpecHandle.Data->SetSetByCallerMagnitude(ECGameplayTags::SetByCaller_DamageMultiplier,	DamageMultiplier);
	DamageSpecHandle.Data->SetSetByCallerMagnitude(ECGameplayTags::SetByCaller_StaminaDamage, StaminaDamage);

	// 데미지를 적용합니다.
	SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
	return true;
}

void UECEnemyDamageAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	EnableVerticalRootMotionMovement();

	// 몽타주에서 이벤트를 수신하기 위해 Task를 등록합니다.
	SetupAttackTasks();

	// 몽타주를 실행시킵니다.
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage, AttackMontageRate);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UECEnemyDamageAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled)
{
	RestoreMovementModeAfterRootMotion(bWasCancelled);
	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled);
}

bool UECEnemyDamageAbility::HasMeaningfulVerticalRootMotion(const UAnimMontage* Montage) const
{
	if (!Montage || !Montage->HasRootMotion())
	{
		return false;
	}

	const double PlayLength = Montage->GetPlayLength();
	if (PlayLength <= UE_KINDA_SMALL_NUMBER)
	{
		return false;
	}

	// The total Z delta of a jump is normally zero, so inspect cumulative
	// transforms throughout the montage rather than only comparing endpoints.
	const int32 NumSamples = FMath::Max(2, FMath::CeilToInt(PlayLength * 30.0));
	double MinimumHeight = 0.0;
	double MaximumHeight = 0.0;
	for (int32 SampleIndex = 1; SampleIndex <= NumSamples; ++SampleIndex)
	{
		const double Time = PlayLength *
			static_cast<double>(SampleIndex) / static_cast<double>(NumSamples);
		const double Height = Montage
			->ExtractRootMotionFromTrackRange(0.f, static_cast<float>(Time))
			.GetTranslation()
			.Z;
		MinimumHeight = FMath::Min(MinimumHeight, Height);
		MaximumHeight = FMath::Max(MaximumHeight, Height);
	}

	return MaximumHeight - MinimumHeight >= VerticalRootMotionThreshold;
}

void UECEnemyDamageAbility::EnableVerticalRootMotionMovement()
{
	bChangedMovementModeForVerticalRootMotion = false;
	VerticalRootMotionMovementComponent.Reset();
	PreviousMovementMode = MOVE_None;
	PreviousCustomMovementMode = 0;

	if (!bAutoEnableVerticalRootMotionMovement ||
		!HasMeaningfulVerticalRootMotion(AttackMontage))
	{
		return;
	}

	const ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	UCharacterMovementComponent* MovementComponent = Character ? Character->GetCharacterMovement() : nullptr;
	if (!MovementComponent || MovementComponent->MovementMode == MOVE_Flying)
	{
		return;
	}

	VerticalRootMotionMovementComponent = MovementComponent;
	PreviousMovementMode = MovementComponent->MovementMode;
	PreviousCustomMovementMode = MovementComponent->CustomMovementMode;
	bChangedMovementModeForVerticalRootMotion = true;
	MovementComponent->SetMovementMode(MOVE_Flying);
}

void UECEnemyDamageAbility::RestoreMovementModeAfterRootMotion(const bool bWasCancelled)
{
	if (!bChangedMovementModeForVerticalRootMotion)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = VerticalRootMotionMovementComponent.Get())
	{
		// Do not overwrite a movement mode deliberately changed by another system.
		if (MovementComponent->MovementMode == MOVE_Flying)
		{
			const EMovementMode RestoreMode =
				bWasCancelled && PreviousMovementMode == MOVE_Walking
					? MOVE_Falling
					: PreviousMovementMode.GetValue();
			MovementComponent->SetMovementMode(RestoreMode, RestoreMode == MOVE_Custom ? PreviousCustomMovementMode : 0);
		}
	}

	bChangedMovementModeForVerticalRootMotion = false;
	VerticalRootMotionMovementComponent.Reset();
	PreviousMovementMode = MOVE_None;
	PreviousCustomMovementMode = 0;
}

void UECEnemyDamageAbility::SetupAttackTasks()
{
	// 자식 클래스에서 구현 필요
}

void UECEnemyDamageAbility::OnMontageFinished()
{	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UECEnemyDamageAbility::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
