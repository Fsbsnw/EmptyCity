// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayCue/ECGameplayCue_CombatImpact.h"

#include "ECGameplayTags.h"
#include "Character/ECCharacterBase.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "Data/Combat/CombatFeedbackDataAsset.h"
#include "Kismet/GameplayStatics.h"

bool UECGameplayCue_CombatImpact::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	// 데이터 에셋 방어 코드
	if (FeedbackDataAsset == nullptr)
	{
		return Super::OnExecute_Implementation(MyTarget, Parameters);
	}

	FGameplayTag WeaponTag = FGameplayTag::EmptyTag;
	FGameplayTag AttackTag = FGameplayTag::EmptyTag;

	// 1. 태그 컨테이너를 순회하며 전투 이벤트에 매칭되는 태그 분류
	for (const FGameplayTag& Tag : Parameters.AggregatedSourceTags)
	{
		if (Tag.MatchesTag(ECGameplayTags::Weapon))
		{
			WeaponTag = Tag;
		}
		else if (Tag.MatchesTag(ECGameplayTags::Attack))
		{
			AttackTag = Tag;
		}
	}

	// 2. 피드백 데이터 가져오기
	FCombatFeedbackResult FeedbackResult = FeedbackDataAsset->GetFeedbackResult(WeaponTag, AttackTag);

	// 3. 사운드 재생
	if (FeedbackResult.HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(MyTarget, FeedbackResult.HitSound, MyTarget->GetActorLocation());
	}

	// 4. 맞은 대상(피격자)의 히트스탑 실행
	if (AECCharacterBase* TargetCharacter = Cast<AECCharacterBase>(MyTarget))
	{
		TargetCharacter->PlayHitStop(FeedbackResult.HitStopTimeDuration, FeedbackResult.HitStopTimeDilation); 
	}

	// 5. 공격 대상(공격자)의 히트스탑 실행
	// if (AECCharacterBase* InstigatorCharacter = Cast<AECCharacterBase>(Parameters.GetInstigator()))
	// {
	//    InstigatorCharacter->PlayHitStop(FeedbackResult.HitStopTimeDuration, FeedbackResult.HitStopTimeDilation); 
	// }

	UE_LOG(LogTemp, Log, TEXT("전투 타격 발생 - 무기: %s, 공격: %s"), *WeaponTag.ToString(), *AttackTag.ToString());
    
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}
