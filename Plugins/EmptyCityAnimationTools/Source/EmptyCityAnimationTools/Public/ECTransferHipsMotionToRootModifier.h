#pragma once

#include "AnimationModifier.h"
#include "Animation/AnimEnums.h"
#include "ECTransferHipsMotionToRootModifier.generated.h"

UENUM(BlueprintType)
enum class EECHipsMotionExtractionMode : uint8
{
	/** Transfer the complete horizontal Hips delta. Best for authored locomotion paths. */
	FullXY UMETA(DisplayName = "Full XY (Legacy)"),

	/** Transfer only the component along a stable forward axis. Best for lunges and jump attacks. */
	ForwardOnly UMETA(DisplayName = "Forward Only"),

	/** Transfer a centered, low-pass-filtered XY trajectory while leaving high-frequency sway on Hips. */
	SmoothedTrajectory UMETA(DisplayName = "Smoothed Trajectory")
};

UENUM(BlueprintType)
enum class EECForwardAxisMode : uint8
{
	/** Use ForwardAxis exactly. This is the safe default for imported combat animations. */
	FixedAxis UMETA(DisplayName = "Fixed Axis"),

	/** Infer a direction from the largest trajectory displacement, using ForwardAxis as fallback. */
	AutoDetectTrajectory UMETA(DisplayName = "Auto Detect Trajectory")
};

UENUM(BlueprintType)
enum class EECRootVerticalMotionPolicy : uint8
{
	/** Keep the root at its first-frame height; all authored vertical pose stays on Hips. */
	KeepOnMotionBone UMETA(DisplayName = "Keep On Motion Bone"),

	/** Keep any vertical animation that was already authored on the root track. */
	PreserveExistingRoot UMETA(DisplayName = "Preserve Existing Root"),

	/** Transfer only the airborne jump arc and return Root Z at the configured landing time. */
	TransferJumpArcReturnAtLanding UMETA(DisplayName = "Transfer Jump Arc (Return At Landing)")
};

/**
 * Transfers a configurable trajectory from a root child (Hips by default) to
 * the skeleton root while keeping semantic forward/up channels separated.
 *
 * By default only the stable forward component is transferred, preventing
 * attack-pose hip sway from becoming lateral character/capsule motion.
 */
UCLASS(DisplayName = "EC Transfer Hips Motion To Root")
class EMPTYCITYANIMATIONTOOLS_API UECTransferHipsMotionToRootModifier : public UAnimationModifier
{
	GENERATED_BODY()

public:
	UECTransferHipsMotionToRootModifier();

	virtual void OnApply_Implementation(UAnimSequence* AnimationSequence) override;
	virtual void OnRevert_Implementation(UAnimSequence* AnimationSequence) override;

protected:
	virtual int32 GetNativeClassRevision() const override;

public:
	/** Direct child of the skeleton root that currently contains locomotion. */
	UPROPERTY(EditAnywhere, Category = "Transfer")
	FName MotionBoneName = TEXT("Hips");

	/** Selects how the source bone trajectory is separated from pose motion. */
	UPROPERTY(EditAnywhere, Category = "Transfer")
	EECHipsMotionExtractionMode ExtractionMode = EECHipsMotionExtractionMode::ForwardOnly;

	/** Controls whether the trajectory uses a fixed or automatically inferred forward axis. */
	UPROPERTY(
		EditAnywhere,
		Category = "Transfer|Direction",
		meta = (EditCondition = "ExtractionMode != EECHipsMotionExtractionMode::FullXY"))
	EECForwardAxisMode ForwardAxisMode = EECForwardAxisMode::FixedAxis;

	/** Component-space fallback direction. Unreal character forward is normally +X. */
	UPROPERTY(
		EditAnywhere,
		Category = "Transfer|Direction",
		meta = (EditCondition = "ExtractionMode != EECHipsMotionExtractionMode::FullXY"))
	FVector ForwardAxis = FVector::ForwardVector;

	/** Forward translation channel in the motion bone's local space. Mixamo-style Hips use +Y. */
	UPROPERTY(EditAnywhere, Category = "Transfer|Direction")
	FVector MotionBoneForwardAxis = FVector(0.0, 1.0, 0.0);

	/** Up translation channel in the motion bone's local space. Mixamo-style Hips use +Z. */
	UPROPERTY(EditAnywhere, Category = "Transfer|Vertical")
	FVector MotionBoneUpAxis = FVector::UpVector;

	/**
	 * Use the first root key as a fixed direction basis. This prevents small
	 * per-frame root rotations from turning forward displacement into side sway.
	 */
	UPROPERTY(EditAnywhere, Category = "Transfer|Direction")
	bool bUseFirstFrameRootBasis = true;

	/** First normalized animation time included in trajectory extraction. */
	UPROPERTY(
		EditAnywhere,
		Category = "Transfer|Range",
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float TransferStartNormalizedTime = 0.0f;

	/** Last normalized animation time included in trajectory extraction. */
	UPROPERTY(
		EditAnywhere,
		Category = "Transfer|Range",
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float TransferEndNormalizedTime = 1.0f;

	/**
	 * Centered smoothing window. Odd values such as 5, 7, or 9 are recommended.
	 * ForwardOnly smooths the forward trajectory; SmoothedTrajectory smooths the XY path.
	 */
	UPROPERTY(
		EditAnywhere,
		Category = "Transfer|Filtering",
		meta = (
			ClampMin = "1",
			UIMin = "1",
			EditCondition = "ExtractionMode != EECHipsMotionExtractionMode::FullXY"))
	int32 SmoothingWindowFrames = 7;

	/** Prevent the primary trajectory component from reversing inside the transfer range. */
	UPROPERTY(
		EditAnywhere,
		Category = "Transfer|Filtering",
		meta = (EditCondition = "ExtractionMode != EECHipsMotionExtractionMode::FullXY"))
	bool bEnforceMonotonicForwardMotion = true;

	/**
	 * Controls vertical ownership. This is deliberately a new property so assets
	 * saved with the old unsafe TransferFromHips value fall back to the safe policy.
	 */
	UPROPERTY(EditAnywhere, Category = "Transfer|Vertical")
	EECRootVerticalMotionPolicy VerticalRootMotionPolicy =
		EECRootVerticalMotionPolicy::TransferJumpArcReturnAtLanding;

	/**
	 * Normalized frame where the feet have landed and Root Z must be back at its
	 * initial height. 0.528571 maps to frame 37 in a 70-frame sequence.
	 */
	UPROPERTY(
		EditAnywhere,
		Category = "Transfer|Vertical",
		meta = (
			ClampMin = "0.0",
			ClampMax = "1.0",
			UIMin = "0.0",
			UIMax = "1.0",
			EditCondition = "VerticalRootMotionPolicy == EECRootVerticalMotionPolicy::TransferJumpArcReturnAtLanding"))
	float VerticalLandingNormalizedTime = 0.528571f;

	/** Enable root-motion extraction on the sequence after transferring keys. */
	UPROPERTY(EditAnywhere, Category = "Root Motion")
	bool bEnableRootMotionAfterApply = true;

private:
	void ResetStoredData();

	UPROPERTY()
	bool bHasStoredData = false;

	UPROPERTY()
	bool bOriginalEnableRootMotion = false;

	UPROPERTY()
	bool bOriginalForceRootLock = false;

	UPROPERTY()
	bool bRootTrackOriginallyExisted = false;

	UPROPERTY()
	int32 OriginalRootTrackIndex = INDEX_NONE;

	UPROPERTY()
	TEnumAsByte<ERootMotionRootLock::Type> OriginalRootMotionRootLock = ERootMotionRootLock::RefPose;

	UPROPERTY()
	FName StoredRootBoneName = NAME_None;

	UPROPERTY()
	FName StoredMotionBoneName = NAME_None;

	UPROPERTY()
	TArray<FVector> OriginalRootPositions;

	UPROPERTY()
	TArray<FQuat> OriginalRootRotations;

	UPROPERTY()
	TArray<FVector> OriginalRootScales;

	UPROPERTY()
	TArray<FVector> OriginalMotionPositions;

	UPROPERTY()
	TArray<FQuat> OriginalMotionRotations;

	UPROPERTY()
	TArray<FVector> OriginalMotionScales;
};
