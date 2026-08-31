#include "CosmeticAnimationTypes.h"

TSubclassOf<UAnimInstance> FECAnimLayerSelectionSet::SelectBestLayer(const FGameplayTagContainer& CosmeticTags) const
{
	// 앞쪽부터 순회하며 태그를 모두 충족하는 첫 규칙을 반환합니다.
	for (const FECAnimLayerSelectionEntry& Rule : LayerRules)
	{
		if ((Rule.Layer != nullptr) && CosmeticTags.HasAll(Rule.RequiredTags))
		{
			return Rule.Layer;
		}
	}
	return DefaultLayer;
}

USkeletalMesh* FECAnimBodyStyleSelectionSet::SelectBestBodyStyle(const FGameplayTagContainer& CosmeticTags) const
{
	// 앞쪽 규칙이 우선합니다. 태그를 모두 충족하는 첫 Mesh를 반환합니다.
	for (const FECAnimBodyStyleSelectionEntry& Rule : MeshRules)
	{
		if ((Rule.Mesh) && CosmeticTags.HasAll(Rule.RequiredTags))
		{
			return Rule.Mesh;
		}
	}

	// 매칭 실패 시 폴백을 반환합니다.
	return DefaultMesh;
}
