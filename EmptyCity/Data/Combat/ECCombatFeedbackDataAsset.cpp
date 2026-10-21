// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/Combat/ECCombatFeedbackDataAsset.h"

namespace
{
	bool DoesTagMatch(const FGameplayTag& ActualTag, const FGameplayTag& ConfiguredTag)
	{
		return !ConfiguredTag.IsValid()
			|| (ActualTag.IsValid() && ActualTag.MatchesTag(ConfiguredTag));
	}

	int32 GetTagMatchScore(const FGameplayTag& ActualTag, const FGameplayTag& ConfiguredTag)
	{
		if (!ConfiguredTag.IsValid())
		{
			return 0;
		}

		return ActualTag.MatchesTagExact(ConfiguredTag) ? 2 : 1;
	}
}

FCombatFeedbackResult UECCombatFeedbackDataAsset::GetFeedbackResult(
	FGameplayTag InSourceTag,
	FGameplayTag InTargetTag,
	FGameplayTag InWeaponTag,
	FGameplayTag InAttackTag) const
{
	const FWeaponCombatFeedbackEntry* BestGroupedMatch = nullptr;
	int32 BestGroupedMatchScore = INDEX_NONE;

	for (const FWeaponCombatFeedbackGroup& Group : WeaponFeedbackGroups)
	{
		if (!DoesTagMatch(InWeaponTag, Group.WeaponTag))
		{
			continue;
		}

		for (const FWeaponCombatFeedbackEntry& Entry : Group.FeedbackEntries)
		{
			if (!DoesTagMatch(InSourceTag, Entry.SourceTag)
				|| !DoesTagMatch(InTargetTag, Entry.TargetTag)
				|| !DoesTagMatch(InAttackTag, Entry.AttackTag))
			{
				continue;
			}

			const int32 MatchScore =
				GetTagMatchScore(InWeaponTag, Group.WeaponTag)
				+ GetTagMatchScore(InSourceTag, Entry.SourceTag)
				+ GetTagMatchScore(InTargetTag, Entry.TargetTag)
				+ GetTagMatchScore(InAttackTag, Entry.AttackTag);

			if (MatchScore > BestGroupedMatchScore)
			{
				BestGroupedMatch = &Entry;
				BestGroupedMatchScore = MatchScore;
			}
		}
	}

	if (BestGroupedMatch)
	{
		return BestGroupedMatch->FeedbackResult;
	}

	return FCombatFeedbackResult();
}

FParryCombatFeedbackResult UECCombatFeedbackDataAsset::GetParryFeedbackResult(
	FGameplayTag InEnemyTypeTag) const
{
	const FParryCombatFeedbackEntry* BestMatch = nullptr;
	int32 BestMatchScore = INDEX_NONE;

	for (const FParryCombatFeedbackEntry& Entry : ParryFeedbackEntries)
	{
		if (!DoesTagMatch(InEnemyTypeTag, Entry.EnemyTypeTag))
		{
			continue;
		}

		const int32 MatchScore = GetTagMatchScore(InEnemyTypeTag, Entry.EnemyTypeTag);
		if (MatchScore > BestMatchScore)
		{
			BestMatch = &Entry;
			BestMatchScore = MatchScore;
		}
	}

	return BestMatch
		? BestMatch->FeedbackResult
		: FParryCombatFeedbackResult();
}
