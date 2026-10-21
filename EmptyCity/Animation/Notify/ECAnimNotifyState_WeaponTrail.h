#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ECAnimNotifyState_WeaponTrail.generated.h"

UENUM(BlueprintType)
enum class EECWeaponTrailSource : uint8
{
	Weapon UMETA(DisplayName="Weapon / Component Default"),
	AnimationMesh UMETA(DisplayName="Kick / Animation Skeletal Mesh")
};

/**
 * 선택한 메시의 Trail VFX를 활성화하는 구간입니다.
 */
UCLASS(meta=(DisplayName="EC Weapon Trail Window"))
class EMPTYCITY_API UECAnimNotifyState_WeaponTrail : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UECAnimNotifyState_WeaponTrail();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	/** Weapon은 무기 메시와 Weapon Trail System을, Kick은 몽타주 메시와 Kick Trail System을 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trail Source")
	EECWeaponTrailSource TrailSource = EECWeaponTrailSource::Weapon;

	/** Kick의 애니메이션 메시에서 트레일 시작 위치로 사용할 본 또는 소켓입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trail Source",
		meta=(EditCondition="TrailSource == EECWeaponTrailSource::AnimationMesh", EditConditionHides))
	FName StartBoneOrSocket = NAME_None;

	/** Kick의 애니메이션 메시에서 트레일 끝 위치로 사용할 본 또는 소켓입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trail Source",
		meta=(EditCondition="TrailSource == EECWeaponTrailSource::AnimationMesh", EditConditionHides))
	FName EndBoneOrSocket = NAME_None;
};
