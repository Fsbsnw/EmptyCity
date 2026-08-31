#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CosmeticAnimationTypes.generated.h"

/** AnimLayer 하나와 그 적용 조건(RequiredTags)을 묶는 규칙입니다. */
USTRUCT(BlueprintType)
struct FECAnimLayerSelectionEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> Layer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer RequiredTags;
};


/** AnimLayer 규칙 묶음입니다. 태그로 최적 Layer를 선택합니다. */
USTRUCT(BlueprintType)
struct FECAnimLayerSelectionSet
{
	GENERATED_BODY();

	/** 태그와 매칭되는 첫 Layer를 반환합니다. 없으면 DefaultLayer를 반환합니다. */
	TSubclassOf<UAnimInstance> SelectBestLayer(const FGameplayTagContainer& CosmeticTags) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FECAnimLayerSelectionEntry> LayerRules;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> DefaultLayer;
};


/** SkeletalMesh 하나와 그 적용 조건(RequiredTags)을 묶는 규칙입니다. */
USTRUCT(BlueprintType)
struct FECAnimBodyStyleSelectionEntry
{
	GENERATED_BODY()

	/** 이 규칙이 선택됐을 때 적용할 Mesh입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMesh> Mesh = nullptr;

	/** 이 Mesh 매칭에 필요한 태그입니다(HasAll 기준). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Cosmetic"))
	FGameplayTagContainer RequiredTags;
};

/** BodyStyle 규칙 묶음입니다. 태그로 최적 Mesh를 선택합니다. */
USTRUCT(BlueprintType)
struct FECAnimBodyStyleSelectionSet
{
	GENERATED_BODY()

	/** 태그와 매칭되는 첫 Mesh를 반환합니다. 없으면 DefaultMesh를 반환합니다. */
	USkeletalMesh* SelectBestBodyStyle(const FGameplayTagContainer& CosmeticTags) const;

	/** 선택 규칙 목록입니다. 앞쪽 규칙이 우선합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FECAnimBodyStyleSelectionEntry> MeshRules;

	/** 매칭 실패 시 폴백 Mesh입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMesh> DefaultMesh = nullptr;

	/** 모든 Mesh에 공통으로 강제 적용하는 PhysicsAsset입니다. */
	UPROPERTY(EditAnywhere)
	TObjectPtr<UPhysicsAsset> ForcedPhysicsAsset = nullptr;
};
