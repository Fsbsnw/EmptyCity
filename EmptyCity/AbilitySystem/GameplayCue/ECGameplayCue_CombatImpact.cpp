// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayCue/ECGameplayCue_CombatImpact.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ECGameplayTags.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/Enemy/ECEnemyCharacterBase.h"
#include "Character/Enemy/Component/HitReactionComponent.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/Combat/ECCombatFeedbackDataAsset.h"
#include "Data/Combat/ECHitVFXAttachmentProfile.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/ECEnemyDataSubsystem.h"

namespace
{
	FGameplayTag ResolveCharacterTypeTag(const AActor* Actor)
	{
		if (const AECEnemyCharacterBase* Enemy = Cast<AECEnemyCharacterBase>(Actor))
		{
			const FGameplayTag EnemyTag =
				UECEnemyDataSubsystem::Get(Enemy).FindEnemyTag(Enemy->GetClass());
			return EnemyTag.IsValid() ? EnemyTag : ECGameplayTags::Character_Type_Enemy;
		}

		const APawn* Pawn = Cast<APawn>(Actor);
		if (Pawn && Pawn->GetPlayerState<AECPlayerState>())
		{
			return ECGameplayTags::Character_Type_Player;
		}

		return FGameplayTag();
	}
}

bool UECGameplayCue_CombatImpact::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (FeedbackDataAsset == nullptr)
	{
		return Super::OnExecute_Implementation(MyTarget, Parameters);
	}

	FGameplayTag SourceTag = ECGameplayTags::Character_Type;
	FGameplayTag TargetTag = ECGameplayTags::Character_Type;
	FGameplayTag WeaponTag = ECGameplayTags::Weapon;
	FGameplayTag AttackTag = ECGameplayTags::Attack;

	// 1. 태그 컨테이너를 순회하며 전투 이벤트에 매칭되는 태그 분류
	for (const FGameplayTag& Tag : Parameters.AggregatedSourceTags)
	{
		if (Tag.MatchesTag(ECGameplayTags::Character_Type))
		{
			SourceTag = Tag;
		}
		else if (Tag.MatchesTag(ECGameplayTags::Weapon))
		{
			WeaponTag = Tag;
		}
		else if (Tag.MatchesTag(ECGameplayTags::Attack))
		{
			AttackTag = Tag;
		}
	}

	const AActor* SourceActor = Parameters.EffectContext.GetInstigator();
	if (const FGameplayTag ResolvedSourceTag = ResolveCharacterTypeTag(SourceActor);
		ResolvedSourceTag.IsValid())
	{
		SourceTag = ResolvedSourceTag;
	}

	if (const FGameplayTag ResolvedTargetTag = ResolveCharacterTypeTag(MyTarget);
		ResolvedTargetTag.IsValid())
	{
		TargetTag = ResolvedTargetTag;
	}

	// 2. 피드백 데이터 가져오기
	FCombatFeedbackResult FeedbackResult = FeedbackDataAsset->GetFeedbackResult(
		SourceTag,
		TargetTag,
		WeaponTag,
		AttackTag);

	const FHitResult* HitResult = Parameters.EffectContext.GetHitResult();
	const FVector ImpactPoint = HitResult ? FVector(HitResult->ImpactPoint) : FVector(Parameters.Location);
	const FVector ImpactNormal = HitResult ? FVector(HitResult->ImpactNormal) : FVector(Parameters.Normal);
	
	// 3. 일반 피격과 가드 성공의 충돌음을 구분합니다.
	USoundBase* ImpactSound = FeedbackResult.HitSound;
	if (const UAbilitySystemComponent* TargetASC =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(MyTarget))
	{
		if (TargetASC->HasMatchingGameplayTag(ECGameplayTags::Status_Guarding)
			&& IsValid(FeedbackDataAsset->GuardFeedback.GuardSound))
		{
			ImpactSound = FeedbackDataAsset->GuardFeedback.GuardSound;
		}
	}

	if (IsValid(ImpactSound))
	{
		UGameplayStatics::PlaySoundAtLocation(MyTarget, ImpactSound, ImpactPoint);
	}

	// 4. 히트스탑 값을 보관한 뒤 피격 몽타주의 Notify 시점에 실행합니다.
	if (MyTarget)
	{
		if (UHitReactionComponent* HitReactionComponent = MyTarget->FindComponentByClass<UHitReactionComponent>())
		{
			HitReactionComponent->QueueHitStop(
				FeedbackResult.HitStopTimeDuration,
				FeedbackResult.HitStopTimeDilation);

			// 피격 몽타주를 실행할 수 없으면 히트스탑이 유실되지 않도록 즉시 실행합니다.
			if (!HitReactionComponent->PlayWeakHitReaction())
			{
				HitReactionComponent->PlayQueuedHitStop();
			}
		}
	}

	// 5. 히트 VFX
	if (FeedbackResult.HitVFX)
	{
		USkeletalMeshComponent* TargetMesh = nullptr;
		if (HitResult)
		{
			TargetMesh = Cast<USkeletalMeshComponent>(HitResult->GetComponent());

			if (TargetMesh == nullptr && HitResult->GetActor())
			{
				TargetMesh = HitResult->GetActor()->FindComponentByClass<USkeletalMeshComponent>();
			}
		}

		if (TargetMesh == nullptr && MyTarget)
		{
			TargetMesh = MyTarget->FindComponentByClass<USkeletalMeshComponent>();
		}

		FName HitBoneName = HitResult ? HitResult->BoneName : NAME_None;
		if (TargetMesh && HitBoneName.IsNone())
		{
			HitBoneName = TargetMesh->FindClosestBone(
				ImpactPoint,
				nullptr,
				0.f,
				false);
		}

		// 실제 피격 본 또는 가장 가까운 본에 프로필 보정을 적용해 부착 실행
		if (TargetMesh && !HitBoneName.IsNone())
		{
			const UECHitVFXAttachmentProfile* AttachmentProfile = nullptr;
			if (const AECEnemyCharacterBase* Enemy = Cast<AECEnemyCharacterBase>(MyTarget))
			{
				AttachmentProfile = UECEnemyDataSubsystem::Get(Enemy)
					.FindHitVFXAttachmentProfile(Enemy->GetClass());
			}

			FECHitVFXAttachmentSettings AttachmentSettings;
			if (AttachmentProfile)
			{
				AttachmentSettings = AttachmentProfile->ResolveSettings(TargetMesh, HitBoneName);
			}
			else
			{
				// 프로필이 없는 기존 적은 이전 위치와 크기를 그대로 유지합니다.
				AttachmentSettings.SurfaceNormalOffset = 0.0f;
			}

			FName AttachBoneName = AttachmentSettings.AttachBone.IsNone()
				? HitBoneName
				: AttachmentSettings.AttachBone;
			if (!TargetMesh->DoesSocketExist(AttachBoneName))
			{
				AttachBoneName = HitBoneName;
			}

			const FTransform AttachBoneTransform =
				TargetMesh->GetSocketTransform(AttachBoneName, RTS_World);
			const FVector SpawnLocation =
				AttachBoneTransform.TransformPosition(AttachmentSettings.LocalOffset)
				+ AttachmentSettings.WorldOffset
				+ ImpactNormal.GetSafeNormal() * AttachmentSettings.SurfaceNormalOffset
				+ FeedbackResult.HitVFXLocationOffset;
			const FRotator SpawnRotation =
				(ImpactNormal.Rotation().Quaternion()
					* AttachmentSettings.RotationOffset.Quaternion()).Rotator();

			UNiagaraComponent* HitVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				FeedbackResult.HitVFX,
				TargetMesh,
				AttachBoneName,
				SpawnLocation,
				SpawnRotation,
				EAttachLocation::KeepWorldPosition,
				true
			);

			if (HitVFXComponent)
			{
				HitVFXComponent->SetWorldScale3D(
					FeedbackResult.VFXScale * AttachmentSettings.ScaleMultiplier);
			}
		}
		// 가까운 본을 찾지 못 했을 경우, 타격 지점에서 실행
		else
		{
			const FRotator ImpactRotation = ImpactNormal.Rotation();
			const FVector SpawnLocation = ImpactPoint
				+ FeedbackResult.HitVFXLocationOffset;

			UNiagaraComponent* HitVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				MyTarget,
				FeedbackResult.HitVFX,
				SpawnLocation,
				ImpactRotation
			);

			if (HitVFXComponent)
			{
				HitVFXComponent->SetWorldScale3D(FeedbackResult.VFXScale);
			}
		}

		// 타격자 : 플레이어인 경우, 캐릭터 바닥에서 웨이브 VFX 실행 임시 구현
		if (SourceTag.MatchesTagExact(ECGameplayTags::Character_Type_Player))
		{
			ACharacter* Player = Cast<ACharacter>(Parameters.EffectContext.GetInstigator());
			if (Player && FeedbackResult.PlayerGroundWaveVFX)
			{
				UCapsuleComponent* Capsule = Player->GetCapsuleComponent();

				const FVector SpawnLocation =
					Capsule->GetComponentLocation()
					- FVector::UpVector * Capsule->GetScaledCapsuleHalfHeight()
					+ FVector::UpVector * 2.f;

				const FRotator SpawnRotation(0.f, Player->GetActorRotation().Yaw,0.f);

				UNiagaraFunctionLibrary::SpawnSystemAttached(
					FeedbackResult.PlayerGroundWaveVFX,
					Capsule,
					NAME_None,
					SpawnLocation,
					SpawnRotation,
					EAttachLocation::KeepWorldPosition,
					true
				);
			}
		}
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("전투 타격 발생 - 공격자: %s, 피격자: %s, 무기: %s, 공격: %s"),
		*SourceTag.ToString(),
		*TargetTag.ToString(),
		*WeaponTag.ToString(),
		*AttackTag.ToString());
    
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}
