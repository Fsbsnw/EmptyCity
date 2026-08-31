// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayCue/ECGameplayCue_CombatImpact.h"

#include "ECGameplayTags.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/Enemy/Component/HitReactionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/Combat/CombatFeedbackDataAsset.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

bool UECGameplayCue_CombatImpact::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (FeedbackDataAsset == nullptr)
	{
		return Super::OnExecute_Implementation(MyTarget, Parameters);
	}

	FGameplayTag SourceTag = ECGameplayTags::Character_Type;
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

	// 2. 피드백 데이터 가져오기
	FCombatFeedbackResult FeedbackResult = FeedbackDataAsset->GetFeedbackResult(SourceTag, WeaponTag, AttackTag);

	const FHitResult* HitResult = Parameters.EffectContext.GetHitResult();
	const FVector ImpactPoint = HitResult ? FVector(HitResult->ImpactPoint) : FVector(Parameters.Location);
	const FVector ImpactNormal = HitResult ? FVector(HitResult->ImpactNormal) : FVector(Parameters.Normal);
	
	// 3. 사운드 재생
	if (FeedbackResult.HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(MyTarget, FeedbackResult.HitSound, ImpactPoint);
	}

	// 4. 맞은 대상(피격자)의 히트스탑 실행
	if (MyTarget)
	{
		if (UHitReactionComponent* HitReactionComponent = MyTarget->FindComponentByClass<UHitReactionComponent>())
		{
			HitReactionComponent->PlayHitStop(FeedbackResult.HitStopTimeDuration, FeedbackResult.HitStopTimeDilation);
			HitReactionComponent->PlayWeakHitReaction();
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

		FVector ClosestBoneLocation = FVector::ZeroVector;
		const FName ClosestBoneName = TargetMesh
			? TargetMesh->FindClosestBone(ImpactPoint, &ClosestBoneLocation, 0.f, false)
			: NAME_None;

		// 피격자 스켈레탈 메시의 가장 가까운 본에 부착 실행
		if (TargetMesh && ClosestBoneName != NAME_None)
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				FeedbackResult.HitVFX,
				TargetMesh,
				ClosestBoneName,
				ClosestBoneLocation,
				ImpactNormal.Rotation(),
				EAttachLocation::KeepWorldPosition,
				true
			);
		}
		// 가까운 본을 찾지 못 했을 경우, 타격 지점에서 실행
		else
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				MyTarget,
				FeedbackResult.HitVFX,
				ImpactPoint,
				ImpactNormal.Rotation()
			);
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

	UE_LOG(LogTemp, Log, TEXT("전투 타격 발생 - 무기: %s, 공격: %s"), *WeaponTag.ToString(), *AttackTag.ToString());
    
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}
