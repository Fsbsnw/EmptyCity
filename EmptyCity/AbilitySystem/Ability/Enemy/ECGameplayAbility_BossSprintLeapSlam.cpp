// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Ability/Enemy/ECGameplayAbility_BossSprintLeapSlam.h"

#include "ECGameplayTags.h"
#include "ECLogChannels.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/Enemy/AI/ECEnemyAIController.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

UECGameplayAbility_BossSprintLeapSlam::UECGameplayAbility_BossSprintLeapSlam()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationPolicy = EAbilityActivationPolicy::ByGameplayEvent;
	ActivationGroup = EAbilityActivationGroup::Exclusive_Blocking;

	FGameplayTagContainer AssetTags = GetAssetTags();
	AssetTags.AddTag(ECGameplayTags::Ability_Type_Skill_Damage_SprintLeapSlam);
	SetAssetTags(AssetTags);
	ActivationOwnedTags.AddTag(ECGameplayTags::Status_Attacking);

	// 전체 몽타주 시작 시점이 아니라 LeapSlam 진입 시점에만 직접 Flying으로 전환합니다.
	bAutoEnableVerticalRootMotionMovement = false;
}

bool UECGameplayAbility_BossSprintLeapSlam::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) || !ActorInfo)
	{
		return false;
	}

	const APawn* AvatarPawn = Cast<APawn>(ActorInfo->AvatarActor.Get());
	const AECEnemyAIController* AIController = AvatarPawn ? Cast<AECEnemyAIController>(AvatarPawn->GetController())	: nullptr;
	const AActor* TargetActor = AIController ? AIController->TargetActor.Get() : nullptr;

	if (!IsValid(TargetActor))
	{
		return false;
	}

	const float Distance = FVector::Dist2D(AvatarPawn->GetActorLocation(), TargetActor->GetActorLocation());
	const float MinimumDistance = FMath::Min(MinActivationDistance, MaxActivationDistance);
	const float MaximumDistance = FMath::Max(MinActivationDistance, MaxActivationDistance);

	return Distance >= MinimumDistance && Distance <= MaximumDistance;
}

void UECGameplayAbility_BossSprintLeapSlam::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	bEndingAbility = false;
	bIssuedSprintMove = false;
	bSetSprintFocus = false;
	bChangedMovementModeForLeap = false;
	FlowPhase = EFlowPhase::None;
	SprintElapsedTime = 0.0f;

	// UECEnemyDamageAbility은 몽타주 시작 즉시 Flying으로 바꾸므로 의도적으로 건너뜁니다.
	UECGameplayAbility::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!AttackMontage || !LeapSlamMontage)
	{
		LOG_ABILITY(TEXT("[%s] Start/Sprint 또는 LeapSlam 몽타주가 설정되지 않았습니다."), *GetName());
		CancelFlow();
		return;
	}

	if (!HasRequiredMontages())
	{
		LOG_ABILITY(
			TEXT("[%s] Start/Sprint 몽타주에 Crouch와 Sprint 섹션이 모두 필요합니다."),
			*GetName());
		CancelFlow();
		return;
	}

	if (!CacheRuntimeReferences(ActorInfo))
	{
		LOG_ABILITY(TEXT("[%s] 보스 AIController, TargetActor 또는 CharacterMovement를 찾지 못했습니다."), *GetName());
		CancelFlow();
		return;
	}

	// Crouch가 시작되는 시점에 코스트와 쿨다운을 확정하여 반복 발동을 막습니다.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelFlow();
		return;
	}

	SetupAttackTasks();
	FlowPhase = EFlowPhase::Crouch;

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("StartSprintMontageTask"),
			AttackMontage,
			AttackMontageRate,
			CrouchSectionName);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::HandleStartSprintMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::HandleStartSprintMontageCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::HandleStartSprintMontageCancelled);
	MontageTask->ReadyForActivation();
	if (bEndingAbility)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FlowTimerHandle,
			this,
			&ThisClass::UpdateFlow,
			FMath::Max(FlowCheckInterval, 0.01f),
			true);
	}
}

void UECGameplayAbility_BossSprintLeapSlam::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled)
{
	if (bEndingAbility)
	{
		return;
	}
	bEndingAbility = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FlowTimerHandle);
	}

	if (bIssuedSprintMove)
	{
		if (AECEnemyAIController* AIController = CachedAIController.Get())
		{
			AIController->StopMovement();
		}
	}
	if (bSetSprintFocus)
	{
		if (AECEnemyAIController* AIController = CachedAIController.Get())
		{
			AIController->ClearFocus(EAIFocusPriority::Gameplay);
		}
	}

	RestoreMovement(bWasCancelled);
	CachedTargetActor.Reset();
	CachedAIController.Reset();
	CachedMovementComponent.Reset();
	FlowPhase = EFlowPhase::None;
	SprintElapsedTime = 0.0f;
	bIssuedSprintMove = false;
	bSetSprintFocus = false;

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled);
}

void UECGameplayAbility_BossSprintLeapSlam::UpdateFlow()
{
	if (bEndingAbility)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	AActor* TargetActor = CachedTargetActor.Get();
	UAnimInstance* AnimInstance = Character && Character->GetMesh()
		? Character->GetMesh()->GetAnimInstance()
		: nullptr;
	if (!Character || !IsValid(TargetActor) || !AnimInstance)
	{
		CancelFlow();
		return;
	}

	const FName CurrentSection = AnimInstance->Montage_GetCurrentSection(AttackMontage);
	if (CurrentSection == SprintSectionName && FlowPhase == EFlowPhase::Crouch)
	{
		if (!BeginSprint())
		{
			CancelFlow();
		}
		return;
	}

	if (FlowPhase != EFlowPhase::Sprint)
	{
		return;
	}

	SprintElapsedTime += FMath::Max(FlowCheckInterval, 0.01f);
	if (SprintElapsedTime >= MaxSprintDuration)
	{
		LOG_ABILITY(TEXT("[%s] Sprint 제한 시간을 초과하여 Ability를 취소합니다."), *GetName());
		CancelFlow();
		return;
	}

	const float Distance = FVector::Dist2D(Character->GetActorLocation(), TargetActor->GetActorLocation());
	if (Distance <= LeapStartDistance)
	{
		BeginLeapSlam();
	}
}

bool UECGameplayAbility_BossSprintLeapSlam::BeginSprint()
{
	AECEnemyAIController* AIController = CachedAIController.Get();
	AActor* TargetActor = CachedTargetActor.Get();
	UCharacterMovementComponent* MovementComponent = CachedMovementComponent.Get();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	UNavigationSystemV1* NavigationSystem = ControlledPawn
		? UNavigationSystemV1::GetCurrent(ControlledPawn->GetWorld())
		: nullptr;
	if (!AIController || !IsValid(TargetActor) || !MovementComponent || !ControlledPawn || !NavigationSystem)
	{
		return false;
	}

	const FNavAgentProperties& NavAgentProperties = MovementComponent->GetNavAgentPropertiesRef();
	FNavLocation ProjectedStartLocation;
	if (!NavigationSystem->ProjectPointToNavigation(
			ControlledPawn->GetActorLocation(),
			ProjectedStartLocation,
			NavigationProjectionExtent,
			&NavAgentProperties))
	{
		LOG_ABILITY(
			TEXT("[%s] 보스 현재 위치를 NavMesh에 투영하지 못했습니다. Location=%s"),
			*GetName(),
			*ControlledPawn->GetActorLocation().ToCompactString());
		return false;
	}

	const float StartProjectionDistance = FVector::Dist2D(
		ControlledPawn->GetActorLocation(),
		ProjectedStartLocation.Location);
	if (StartProjectionDistance > MaxStartNavProjectionDistance)
	{
		LOG_ABILITY(
			TEXT("[%s] 보스가 NavMesh에서 너무 멀리 벗어나 Sprint를 취소합니다. Distance=%.1f Location=%s Projected=%s"),
			*GetName(),
			StartProjectionDistance,
			*ControlledPawn->GetActorLocation().ToCompactString(),
			*ProjectedStartLocation.Location.ToCompactString());
		return false;
	}

	FNavLocation ProjectedTargetLocation;
	if (!NavigationSystem->ProjectPointToNavigation(
			TargetActor->GetActorLocation(),
			ProjectedTargetLocation,
			NavigationProjectionExtent,
			&NavAgentProperties))
	{
		LOG_ABILITY(
			TEXT("[%s] 플레이어 위치를 NavMesh에 투영하지 못했습니다. Location=%s"),
			*GetName(),
			*TargetActor->GetActorLocation().ToCompactString());
		return false;
	}

	FlowPhase = EFlowPhase::Sprint;
	SprintElapsedTime = 0.0f;
	MovementComponent->MaxWalkSpeed = SprintSpeed;
	AIController->SetFocus(TargetActor, EAIFocusPriority::Gameplay);
	bSetSprintFocus = true;

	const EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
		ProjectedTargetLocation.Location,
		LeapStartDistance,
		false,
		true,
		false,
		true,
		nullptr,
		false);

	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		LOG_ABILITY(TEXT("[%s] Sprint MoveTo 요청이 실패했습니다."), *GetName());
		return false;
	}

	if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		BeginLeapSlam();
		return true;
	}

	bIssuedSprintMove = true;
	return true;
}

void UECGameplayAbility_BossSprintLeapSlam::BeginLeapSlam()
{
	if (FlowPhase == EFlowPhase::LeapSlam || bEndingAbility)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	AActor* TargetActor = CachedTargetActor.Get();
	UCharacterMovementComponent* MovementComponent = CachedMovementComponent.Get();
	if (!Character || !IsValid(TargetActor) || !MovementComponent)
	{
		CancelFlow();
		return;
	}

	FlowPhase = EFlowPhase::LeapSlam;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FlowTimerHandle);
	}

	if (AECEnemyAIController* AIController = CachedAIController.Get())
	{
		AIController->StopMovement();
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}
	bIssuedSprintMove = false;
	bSetSprintFocus = false;
	MovementComponent->MaxWalkSpeed = PreviousMaxWalkSpeed;

	FVector ToTarget = TargetActor->GetActorLocation() - Character->GetActorLocation();
	ToTarget.Z = 0.0f;
	if (!ToTarget.IsNearlyZero())
	{
		FRotator FacingRotation = Character->GetActorRotation();
		FacingRotation.Yaw = ToTarget.Rotation().Yaw;
		Character->SetActorRotation(FacingRotation);
	}

	if (MovementComponent->MovementMode != MOVE_Flying)
	{
		MovementComponent->SetMovementMode(MOVE_Flying);
		bChangedMovementModeForLeap = true;
	}

	// 같은 Slot에서 새 몽타주를 재생하면 Start/Sprint 몽타주는 Interrupted됩니다.
	// FlowPhase를 먼저 LeapSlam으로 변경했으므로 해당 Interrupted는 정상 전환으로 무시됩니다.
	UAbilityTask_PlayMontageAndWait* LeapMontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("LeapSlamMontageTask"),
			LeapSlamMontage,
			LeapSlamMontageRate);

	LeapMontageTask->OnCompleted.AddDynamic(this, &ThisClass::HandleLeapSlamMontageCompleted);
	LeapMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::HandleLeapSlamMontageCancelled);
	LeapMontageTask->OnCancelled.AddDynamic(this, &ThisClass::HandleLeapSlamMontageCancelled);
	LeapMontageTask->ReadyForActivation();
}

void UECGameplayAbility_BossSprintLeapSlam::CancelFlow()
{
	if (!bEndingAbility)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

void UECGameplayAbility_BossSprintLeapSlam::RestoreMovement(const bool bWasCancelled)
{
	UCharacterMovementComponent* MovementComponent = CachedMovementComponent.Get();
	if (!MovementComponent)
	{
		return;
	}

	MovementComponent->MaxWalkSpeed = PreviousMaxWalkSpeed;
	if (bChangedMovementModeForLeap && MovementComponent->MovementMode == MOVE_Flying)
	{
		const EMovementMode RestoreMode =
			bWasCancelled && PreviousMovementMode == MOVE_Walking
				? MOVE_Falling
				: PreviousMovementMode.GetValue();
		MovementComponent->SetMovementMode(
			RestoreMode,
			RestoreMode == MOVE_Custom ? PreviousCustomMovementMode : 0);
	}

	bChangedMovementModeForLeap = false;
}

bool UECGameplayAbility_BossSprintLeapSlam::CacheRuntimeReferences(const FGameplayAbilityActorInfo* ActorInfo)
{
	ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	AECEnemyAIController* AIController = Character
		? Cast<AECEnemyAIController>(Character->GetController())
		: nullptr;
	AActor* TargetActor = AIController ? AIController->TargetActor.Get() : nullptr;
	UCharacterMovementComponent* MovementComponent = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Character || !AIController || !IsValid(TargetActor) || !MovementComponent)
	{
		return false;
	}

	CachedTargetActor = TargetActor;
	CachedAIController = AIController;
	CachedMovementComponent = MovementComponent;
	PreviousMovementMode = MovementComponent->MovementMode;
	PreviousCustomMovementMode = MovementComponent->CustomMovementMode;
	PreviousMaxWalkSpeed = MovementComponent->MaxWalkSpeed;
	return true;
}

bool UECGameplayAbility_BossSprintLeapSlam::HasRequiredMontages() const
{
	return AttackMontage && LeapSlamMontage &&
		AttackMontage->GetSectionIndex(CrouchSectionName) != INDEX_NONE &&
		AttackMontage->GetSectionIndex(SprintSectionName) != INDEX_NONE;
}

void UECGameplayAbility_BossSprintLeapSlam::HandleStartSprintMontageCompleted()
{
	if (!bEndingAbility && FlowPhase != EFlowPhase::LeapSlam)
	{
		LOG_ABILITY(TEXT("[%s] LeapSlam 진입 전에 Start/Sprint 몽타주가 종료되었습니다."), *GetName());
		CancelFlow();
	}
}

void UECGameplayAbility_BossSprintLeapSlam::HandleStartSprintMontageCancelled()
{
	if (!bEndingAbility && FlowPhase != EFlowPhase::LeapSlam)
	{
		// Melee 부모의 패링 처리(패링 Ability에 종료를 위임)를 그대로 유지합니다.
		UECEnemyDamageAbility_Melee::OnMontageCancelled();
	}
}

void UECGameplayAbility_BossSprintLeapSlam::HandleLeapSlamMontageCompleted()
{
	if (!bEndingAbility)
	{
		OnMontageFinished();
	}
}

void UECGameplayAbility_BossSprintLeapSlam::HandleLeapSlamMontageCancelled()
{
	if (!bEndingAbility)
	{
		// Melee 부모의 패링 처리(패링 Ability에 종료를 위임)를 그대로 유지합니다.
		UECEnemyDamageAbility_Melee::OnMontageCancelled();
	}
}
