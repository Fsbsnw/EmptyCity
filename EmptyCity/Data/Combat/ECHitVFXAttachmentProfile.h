#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ECHitVFXAttachmentProfile.generated.h"

class USkeletalMeshComponent;

/** 피격 이펙트를 본에 부착할 때 적용할 위치와 회전 보정값입니다. */
USTRUCT(BlueprintType)
struct FECHitVFXAttachmentSettings
{
	GENERATED_BODY()

	/** 비어 있으면 실제 피격 본을 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attachment")
	FName AttachBone = NAME_None;

	/** 부착할 본의 로컬 축을 기준으로 적용할 위치 오프셋입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attachment", meta = (Units = "cm"))
	FVector LocalOffset = FVector::ZeroVector;

	/** 본의 회전과 관계없이 월드 축 기준으로 적용할 위치 오프셋입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attachment", meta = (Units = "cm"))
	FVector WorldOffset = FVector::ZeroVector;

	/** 실제 피격 표면의 바깥 방향으로 이펙트를 밀어낼 거리입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attachment", meta = (ClampMin = "0.0", Units = "cm"))
	float SurfaceNormalOffset = 3.0f;

	/** 피격 표면 방향을 기준으로 추가 적용할 회전값입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attachment")
	FRotator RotationOffset = FRotator::ZeroRotator;

	/** CombatFeedback에 설정된 이펙트 크기에 곱할 값입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attachment")
	FVector ScaleMultiplier = FVector::OneVector;
};

/** 특정 피격 본들을 다른 본과 보정값으로 변환하는 규칙입니다. */
USTRUCT(BlueprintType)
struct FECHitVFXBoneOverride
{
	GENERATED_BODY()

	/** 이 본에 피격됐을 때 해당 규칙을 적용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bone Override")
	FName SourceBone = NAME_None;

	/** 활성화하면 Source Bone 아래의 모든 자식 본에도 해당 규칙을 적용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bone Override")
	bool bIncludeDescendantBones = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bone Override")
	FECHitVFXAttachmentSettings Settings;
};

/** 같은 스켈레톤을 사용하는 적들이 공유하는 피격 이펙트 부착 규칙입니다. */
UCLASS(BlueprintType)
class EMPTYCITY_API UECHitVFXAttachmentProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 별도 본 규칙이 없을 때 적용할 기본 보정값입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit VFX Attachment")
	FECHitVFXAttachmentSettings DefaultSettings;

	/** 원래 피격 본을 기준으로 검색할 예외 규칙입니다. 먼저 일치한 규칙을 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit VFX Attachment")
	TArray<FECHitVFXBoneOverride> BoneOverrides;

	/** 정확히 일치하는 본을 우선하고, 이후 가장 가까운 부모의 자식 포함 규칙을 검색합니다. */
	const FECHitVFXAttachmentSettings& ResolveSettings(
		const USkeletalMeshComponent* TargetMesh,
		FName HitBone) const;
};
