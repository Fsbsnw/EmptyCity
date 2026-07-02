// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/Cutscene/CutsceneDataAsset.h"

bool UCutsceneDataAsset::FindCutsceneData(FGameplayTag InTag, FCutsceneData& OutData) const
{
	for (const auto& Row : CutsceneDataRows)
	{
		if (Row.CutsceneTag.MatchesTagExact(InTag))
		{
			OutData = Row.CutsceneData;
			return true;
		}
	}
	return false;
}