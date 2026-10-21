#include "AbilitySystem/GameplayCue/ECGameplayCue_ParryImpact.h"

#include "ECGameplayTags.h"
#include "Character/Enemy/ECEnemyCharacterBase.h"
#include "Character/Player/ECPlayer.h"
#include "Data/Combat/ECCombatFeedbackDataAsset.h"
#include "Equipment/Weapon/ECWeaponInstance.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Subsystem/ECEnemyDataSubsystem.h"

bool UECGameplayCue_ParryImpact::OnExecute_Implementation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters) const
{
	if (!IsValid(MyTarget) || !IsValid(FeedbackDataAsset))
	{
		return Super::OnExecute_Implementation(MyTarget, Parameters);
	}

	FGameplayTag EnemyTypeTag = ECGameplayTags::Character_Type_Enemy;
	if (const AECEnemyCharacterBase* Enemy =
		Cast<AECEnemyCharacterBase>(Parameters.EffectContext.GetInstigator()))
	{
		const FGameplayTag ResolvedTag =
			UECEnemyDataSubsystem::Get(Enemy).FindEnemyTag(Enemy->GetClass());
		if (ResolvedTag.IsValid())
		{
			EnemyTypeTag = ResolvedTag;
		}
	}

	const FParryCombatFeedbackResult FeedbackResult =
		FeedbackDataAsset->GetParryFeedbackResult(EnemyTypeTag);
	const FHitResult* HitResult = Parameters.EffectContext.GetHitResult();
	const FVector ImpactPoint = HitResult
		? FVector(HitResult->ImpactPoint)
		: FVector(Parameters.Location);
	const FVector ImpactNormal = HitResult
		? FVector(HitResult->ImpactNormal)
		: FVector(Parameters.Normal);
	FVector ParryVFXAnchorLocation = ImpactPoint;
	if (const AECPlayer* Player = Cast<AECPlayer>(MyTarget))
	{
		if (const UECWeaponInstance* WeaponInstance = Player->GetEquippedWeapon())
		{
			FTransform WeaponMidTransform;
			if (WeaponInstance->GetParryImpactTransform(WeaponMidTransform))
			{
				ParryVFXAnchorLocation = WeaponMidTransform.GetLocation();
			}
		}
	}

	const FVector SpawnLocation =
		ParryVFXAnchorLocation + FeedbackResult.ParryVFXLocationOffset;

	if (IsValid(FeedbackResult.ParrySound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			MyTarget,
			FeedbackResult.ParrySound,
			ImpactPoint);
	}

	if (IsValid(FeedbackResult.ParryVFX))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			MyTarget,
			FeedbackResult.ParryVFX,
			SpawnLocation,
			ImpactNormal.Rotation(),
			FeedbackResult.ParryVFXScale);
	}

	return Super::OnExecute_Implementation(MyTarget, Parameters);
}
