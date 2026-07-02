// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/Combat/CombatFeedbackDataAsset.h"

FCombatFeedbackResult UCombatFeedbackDataAsset::GetFeedbackResult(FGameplayTag InWeaponTag,	FGameplayTag InAttackTag) const
{
	// 정확히 일치하는 행이 있는지 검사
	for (const FCombatFeedbackRow& Row : CombatFeedbackRows)
	{
		if (Row.WeaponTag.MatchesTagExact(InWeaponTag) && Row.AttackTag.MatchesTagExact(InAttackTag))
		{
			return Row.FeedbackResult;
		}
	}
	
	// 아무것도 찾지 못했다면 빈 구조체 반환
	return FCombatFeedbackResult();
}