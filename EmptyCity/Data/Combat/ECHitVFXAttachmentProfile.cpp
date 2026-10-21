#include "Data/Combat/ECHitVFXAttachmentProfile.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"

const FECHitVFXAttachmentSettings& UECHitVFXAttachmentProfile::ResolveSettings(
	const USkeletalMeshComponent* TargetMesh,
	FName HitBone) const
{
	if (HitBone.IsNone())
	{
		return DefaultSettings;
	}

	// 자식 포함 여부와 관계없이 정확히 지정한 본의 규칙을 가장 먼저 사용합니다.
	for (const FECHitVFXBoneOverride& BoneOverride : BoneOverrides)
	{
		if (BoneOverride.SourceBone == HitBone)
		{
			return BoneOverride.Settings;
		}
	}

	const USkeletalMesh* SkeletalMesh = IsValid(TargetMesh)
		? TargetMesh->GetSkeletalMeshAsset()
		: nullptr;
	if (!IsValid(SkeletalMesh))
	{
		return DefaultSettings;
	}

	const FReferenceSkeleton& ReferenceSkeleton = SkeletalMesh->GetRefSkeleton();
	int32 BoneIndex = ReferenceSkeleton.FindBoneIndex(HitBone);
	while (BoneIndex != INDEX_NONE)
	{
		BoneIndex = ReferenceSkeleton.GetParentIndex(BoneIndex);
		if (BoneIndex == INDEX_NONE)
		{
			break;
		}

		const FName ParentBone = ReferenceSkeleton.GetBoneName(BoneIndex);
		for (const FECHitVFXBoneOverride& BoneOverride : BoneOverrides)
		{
			if (BoneOverride.bIncludeDescendantBones
				&& BoneOverride.SourceBone == ParentBone)
			{
				return BoneOverride.Settings;
			}
		}
	}

	return DefaultSettings;
}
