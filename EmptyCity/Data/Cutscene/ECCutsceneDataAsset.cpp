// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/Cutscene/ECCutsceneDataAsset.h"

bool UECCutsceneDataAsset::FindCutsceneData(FGameplayTag InTag, FCutsceneData& OutData) const
{
	for (const FCutsceneDataRow& Row : CutsceneDataRows)
	{
		if (Row.CutsceneTag.MatchesTagExact(InTag))
		{
			OutData = Row.CutsceneData;
			return true;
		}
	}
	return false;
}
