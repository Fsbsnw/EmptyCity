#include "ECTransferHipsMotionToRootModifier.h"

#include "Animation/AnimData/AnimDataModel.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "EngineLogs.h"

#define LOCTEXT_NAMESPACE "ECTransferHipsMotionToRootModifier"

namespace
{
	struct FBoneTrackKeys
	{
		TArray<FVector> Positions;
		TArray<FQuat> Rotations;
		TArray<FVector> Scales;

		void Reserve(const int32 NumKeys)
		{
			Positions.Reserve(NumKeys);
			Rotations.Reserve(NumKeys);
			Scales.Reserve(NumKeys);
		}

		void Add(const FTransform& Transform)
		{
			Positions.Add(Transform.GetLocation());
			Rotations.Add(Transform.GetRotation());
			Scales.Add(Transform.GetScale3D());
		}
	};

	struct FNamedBoneTrackKeys
	{
		FName Name = NAME_None;
		FBoneTrackKeys Keys;
	};

	bool CaptureBoneTracks(
		const IAnimationDataModel* Model,
		TArray<FNamedBoneTrackKeys>& OutTracks)
	{
		if (!Model)
		{
			return false;
		}

		TArray<FName> TrackNames;
		Model->GetBoneTrackNames(TrackNames);
		OutTracks.Reset(TrackNames.Num());
		for (const FName TrackName : TrackNames)
		{
			TArray<FTransform> Transforms;
			Model->GetBoneTrackTransforms(TrackName, Transforms);
			if (Transforms.IsEmpty())
			{
				return false;
			}

			FNamedBoneTrackKeys& Track = OutTracks.AddDefaulted_GetRef();
			Track.Name = TrackName;
			Track.Keys.Reserve(Transforms.Num());
			for (const FTransform& Transform : Transforms)
			{
				Track.Keys.Add(Transform);
			}
		}

		return true;
	}

	bool RebuildBoneTracks(
		IAnimationDataController& Controller,
		const TArray<FNamedBoneTrackKeys>& Tracks,
		const bool bShouldTransact)
	{
		Controller.RemoveAllBoneTracks(bShouldTransact);
		for (const FNamedBoneTrackKeys& Track : Tracks)
		{
			if (!Controller.AddBoneCurve(Track.Name, bShouldTransact) ||
				!Controller.SetBoneTrackKeys(
					Track.Name,
					Track.Keys.Positions,
					Track.Keys.Rotations,
					Track.Keys.Scales,
					bShouldTransact))
			{
				return false;
			}
		}

		return true;
	}

	bool NormalizeHorizontal(FVector& Vector)
	{
		Vector.Z = 0.0;
		return Vector.Normalize();
	}

	bool ResolveForwardAxis(
		const TArray<FVector>& RawOffsets,
		const int32 StartKey,
		const int32 EndKey,
		const bool bAutoDetect,
		const FVector& FallbackAxis,
		FVector& OutForwardAxis)
	{
		OutForwardAxis = FVector::ZeroVector;
		if (bAutoDetect)
		{
			double MaximumDistanceSquared = 0.0;
			for (int32 KeyIndex = StartKey; KeyIndex <= EndKey; ++KeyIndex)
			{
				FVector Candidate = RawOffsets[KeyIndex];
				Candidate.Z = 0.0;
				const double DistanceSquared = Candidate.SizeSquared();
				if (DistanceSquared > MaximumDistanceSquared)
				{
					MaximumDistanceSquared = DistanceSquared;
					OutForwardAxis = Candidate;
				}
			}

			if (MaximumDistanceSquared > FMath::Square(static_cast<double>(UE_KINDA_SMALL_NUMBER)) &&
				NormalizeHorizontal(OutForwardAxis))
			{
				return true;
			}
		}

		OutForwardAxis = FallbackAxis;
		return NormalizeHorizontal(OutForwardAxis);
	}

	void SmoothTrajectoryRange(
		TArray<FVector>& InOutOffsets,
		const int32 StartKey,
		const int32 EndKey,
		const int32 WindowFrames)
	{
		if (WindowFrames <= 1 || EndKey - StartKey < 2)
		{
			return;
		}

		const int32 Radius = FMath::Max(1, WindowFrames / 2);
		const TArray<FVector> SourceOffsets = InOutOffsets;
		for (int32 KeyIndex = StartKey; KeyIndex <= EndKey; ++KeyIndex)
		{
			const int32 FirstSample = FMath::Max(StartKey, KeyIndex - Radius);
			const int32 LastSample = FMath::Min(EndKey, KeyIndex + Radius);
			FVector Sum = FVector::ZeroVector;
			for (int32 SampleIndex = FirstSample; SampleIndex <= LastSample; ++SampleIndex)
			{
				Sum += SourceOffsets[SampleIndex];
			}

			InOutOffsets[KeyIndex] = Sum / static_cast<double>(LastSample - FirstSample + 1);
		}

		// A centered filter changes the endpoints. Remove that drift gradually so
		// the selected range still starts and finishes at the authored positions.
		const FVector StartError = InOutOffsets[StartKey] - SourceOffsets[StartKey];
		const FVector EndError = InOutOffsets[EndKey] - SourceOffsets[EndKey];
		const double KeySpan = static_cast<double>(EndKey - StartKey);
		for (int32 KeyIndex = StartKey; KeyIndex <= EndKey; ++KeyIndex)
		{
			const double Alpha = static_cast<double>(KeyIndex - StartKey) / KeySpan;
			InOutOffsets[KeyIndex] -= FMath::Lerp(StartError, EndError, Alpha);
		}
	}

	void EnforceMonotonicForwardMotion(
		TArray<FVector>& InOutOffsets,
		const int32 StartKey,
		const int32 EndKey,
		const FVector& ForwardAxis)
	{
		const double StartDistance = FVector::DotProduct(InOutOffsets[StartKey], ForwardAxis);
		double EndDistance = FVector::DotProduct(InOutOffsets[EndKey], ForwardAxis);
		if (FMath::IsNearlyEqual(StartDistance, EndDistance))
		{
			for (int32 KeyIndex = StartKey; KeyIndex <= EndKey; ++KeyIndex)
			{
				const double Candidate = FVector::DotProduct(InOutOffsets[KeyIndex], ForwardAxis);
				if (FMath::Abs(Candidate - StartDistance) > FMath::Abs(EndDistance - StartDistance))
				{
					EndDistance = Candidate;
				}
			}
		}

		const double DirectionSign = EndDistance >= StartDistance ? 1.0 : -1.0;
		double PreviousSignedDistance = DirectionSign * StartDistance;
		for (int32 KeyIndex = StartKey + 1; KeyIndex <= EndKey; ++KeyIndex)
		{
			const double OriginalDistance = FVector::DotProduct(InOutOffsets[KeyIndex], ForwardAxis);
			const double SignedDistance = DirectionSign * OriginalDistance;
			const double MonotonicDistance = FMath::Max(PreviousSignedDistance, SignedDistance);
			const double CorrectedDistance = DirectionSign * MonotonicDistance;
			InOutOffsets[KeyIndex] += ForwardAxis * (CorrectedDistance - OriginalDistance);
			PreviousSignedDistance = MonotonicDistance;
		}
	}
}

UECTransferHipsMotionToRootModifier::UECTransferHipsMotionToRootModifier()
{
	bReapplyPostOwnerChange = false;
}

void UECTransferHipsMotionToRootModifier::OnApply_Implementation(UAnimSequence* AnimationSequence)
{
	ResetStoredData();

	if (!AnimationSequence)
	{
		UE_LOG(LogAnimation, Error, TEXT("Transfer Hips Motion To Root failed: animation is null."));
		return;
	}

	const USkeleton* Skeleton = AnimationSequence->GetSkeleton();
	const IAnimationDataModel* Model = AnimationSequence->GetDataModel();
	if (!Skeleton || !Model)
	{
		UE_LOG(
			LogAnimation,
			Error,
			TEXT("Transfer Hips Motion To Root failed: %s has no valid skeleton or data model."),
			*GetNameSafe(AnimationSequence));
		return;
	}

	const FReferenceSkeleton& ReferenceSkeleton = Skeleton->GetReferenceSkeleton();
	if (ReferenceSkeleton.GetNum() == 0)
	{
		UE_LOG(
			LogAnimation,
			Error,
			TEXT("Transfer Hips Motion To Root failed: %s has an empty reference skeleton."),
			*GetNameSafe(AnimationSequence));
		return;
	}

	const FName RootBoneName = ReferenceSkeleton.GetBoneName(0);
	const int32 MotionBoneIndex = ReferenceSkeleton.FindBoneIndex(MotionBoneName);
	if (MotionBoneIndex == INDEX_NONE)
	{
		UE_LOG(
			LogAnimation,
			Error,
			TEXT("Transfer Hips Motion To Root failed: bone '%s' does not exist in %s."),
			*MotionBoneName.ToString(),
			*GetNameSafe(Skeleton));
		return;
	}

	if (ReferenceSkeleton.GetParentIndex(MotionBoneIndex) != 0)
	{
		UE_LOG(
			LogAnimation,
			Error,
			TEXT("Transfer Hips Motion To Root failed: '%s' must be a direct child of root '%s' in %s."),
			*MotionBoneName.ToString(),
			*RootBoneName.ToString(),
			*GetNameSafe(Skeleton));
		return;
	}

	if (!Model->IsValidBoneTrackName(MotionBoneName))
	{
		UE_LOG(
			LogAnimation,
			Error,
			TEXT("Transfer Hips Motion To Root failed: %s does not contain a track for '%s'."),
			*GetNameSafe(AnimationSequence),
			*MotionBoneName.ToString());
		return;
	}

	const int32 NumKeys = Model->GetNumberOfKeys();
	if (NumKeys <= 0)
	{
		UE_LOG(
			LogAnimation,
			Error,
			TEXT("Transfer Hips Motion To Root failed: %s has no animation keys."),
			*GetNameSafe(AnimationSequence));
		return;
	}

	const int32 LastKeyIndex = NumKeys - 1;
	const int32 TransferStartKey = FMath::Clamp(
		FMath::RoundToInt(TransferStartNormalizedTime * LastKeyIndex),
		0,
		LastKeyIndex);
	const int32 TransferEndKey = FMath::Clamp(
		FMath::RoundToInt(TransferEndNormalizedTime * LastKeyIndex),
		0,
		LastKeyIndex);
	if (TransferEndKey <= TransferStartKey)
	{
		UE_LOG(
			LogAnimation,
			Error,
			TEXT("Transfer Hips Motion To Root failed: transfer range %.3f-%.3f is empty in %s."),
			TransferStartNormalizedTime,
			TransferEndNormalizedTime,
			*GetNameSafe(AnimationSequence));
		return;
	}

	StoredRootBoneName = RootBoneName;
	StoredMotionBoneName = MotionBoneName;
	bOriginalEnableRootMotion = AnimationSequence->bEnableRootMotion;
	bOriginalForceRootLock = AnimationSequence->bForceRootLock;
	OriginalRootMotionRootLock = AnimationSequence->RootMotionRootLock;
	TArray<FName> OriginalBoneTrackNames;
	Model->GetBoneTrackNames(OriginalBoneTrackNames);
	OriginalRootTrackIndex = OriginalBoneTrackNames.IndexOfByKey(RootBoneName);
	bRootTrackOriginallyExisted = OriginalRootTrackIndex != INDEX_NONE;

	// Keep a complete local snapshot so any failure after the controller starts
	// mutating tracks can be rolled back immediately. The modifier framework's
	// transaction provides a second safety net outside this function.
	TArray<FNamedBoneTrackKeys> OriginalBoneTracks;
	if (!CaptureBoneTracks(Model, OriginalBoneTracks))
	{
		UE_LOG(
			LogAnimation,
			Error,
			TEXT("Transfer Hips Motion To Root failed: could not capture source tracks in %s."),
			*GetNameSafe(AnimationSequence));
		ResetStoredData();
		return;
	}

	OriginalRootPositions.Reserve(NumKeys);
	OriginalRootRotations.Reserve(NumKeys);
	OriginalRootScales.Reserve(NumKeys);
	OriginalMotionPositions.Reserve(NumKeys);
	OriginalMotionRotations.Reserve(NumKeys);
	OriginalMotionScales.Reserve(NumKeys);

	TArray<FTransform> OriginalRootTransforms;
	TArray<FTransform> OriginalMotionTransforms;
	OriginalRootTransforms.Reserve(NumKeys);
	OriginalMotionTransforms.Reserve(NumKeys);

	for (int32 KeyIndex = 0; KeyIndex < NumKeys; ++KeyIndex)
	{
		const FTransform RootTransform = bRootTrackOriginallyExisted
			? Model->EvaluateBoneTrackTransform(RootBoneName, KeyIndex, EAnimInterpolationType::Step)
			: ReferenceSkeleton.GetRefBonePose()[0];
		const FTransform MotionTransform =
			Model->EvaluateBoneTrackTransform(MotionBoneName, KeyIndex, EAnimInterpolationType::Step);

		OriginalRootTransforms.Add(RootTransform);
		OriginalMotionTransforms.Add(MotionTransform);

		OriginalRootPositions.Add(RootTransform.GetLocation());
		OriginalRootRotations.Add(RootTransform.GetRotation());
		OriginalRootScales.Add(RootTransform.GetScale3D());
		OriginalMotionPositions.Add(MotionTransform.GetLocation());
		OriginalMotionRotations.Add(MotionTransform.GetRotation());
		OriginalMotionScales.Add(MotionTransform.GetScale3D());
	}

	bHasStoredData = true;

	const FVector TransferStartMotionLocation =
		OriginalMotionTransforms[TransferStartKey].GetLocation();
	const FQuat FixedRootBasis = OriginalRootTransforms[0].GetRotation();
	FVector ResolvedMotionUpAxis = MotionBoneUpAxis;
	if (!ResolvedMotionUpAxis.Normalize())
	{
		UE_LOG(
			LogAnimation,
			Error,
			TEXT("Transfer Hips Motion To Root failed: MotionBoneUpAxis is invalid in %s."),
			*GetNameSafe(AnimationSequence));
		ResetStoredData();
		return;
	}

	FVector ResolvedMotionForwardAxis = MotionBoneForwardAxis;
	ResolvedMotionForwardAxis -= ResolvedMotionUpAxis * FVector::DotProduct(
		ResolvedMotionForwardAxis,
		ResolvedMotionUpAxis);
	if (!ResolvedMotionForwardAxis.Normalize())
	{
		UE_LOG(
			LogAnimation,
			Error,
			TEXT("Transfer Hips Motion To Root failed: MotionBoneForwardAxis must not be parallel to MotionBoneUpAxis in %s."),
			*GetNameSafe(AnimationSequence));
		ResetStoredData();
		return;
	}

	const bool bExtractVerticalMotion =
		VerticalRootMotionPolicy ==
		EECRootVerticalMotionPolicy::TransferJumpArcReturnAtLanding;
	TArray<FVector> RawOffsets;
	RawOffsets.Reserve(NumKeys);
	for (int32 KeyIndex = 0; KeyIndex < NumKeys; ++KeyIndex)
	{
		const FVector MotionDeltaInMotionBoneSpace =
			OriginalMotionTransforms[KeyIndex].GetLocation() - TransferStartMotionLocation;
		const double MotionForwardDistance = FVector::DotProduct(
			MotionDeltaInMotionBoneSpace,
			ResolvedMotionForwardAxis);
		const double MotionVerticalDistance = FVector::DotProduct(
			MotionDeltaInMotionBoneSpace,
			ResolvedMotionUpAxis);
		const FVector MotionHorizontalDelta =
			MotionDeltaInMotionBoneSpace -
			ResolvedMotionUpAxis * MotionVerticalDistance;
		const FVector MotionTrajectoryDelta =
			ExtractionMode == EECHipsMotionExtractionMode::ForwardOnly
				? ResolvedMotionForwardAxis * MotionForwardDistance
				: MotionHorizontalDelta;
		FVector Offset = bUseFirstFrameRootBasis
			? FixedRootBasis.RotateVector(MotionTrajectoryDelta)
			: OriginalRootTransforms[KeyIndex].TransformVectorNoScale(MotionTrajectoryDelta);
		Offset.Z = bExtractVerticalMotion ? MotionVerticalDistance : 0.0;
		RawOffsets.Add(Offset);
	}

	FVector ResolvedForwardAxis = ForwardAxis;
	if (ExtractionMode != EECHipsMotionExtractionMode::FullXY &&
		!ResolveForwardAxis(
			RawOffsets,
			TransferStartKey,
			TransferEndKey,
			ForwardAxisMode == EECForwardAxisMode::AutoDetectTrajectory,
			ForwardAxis,
			ResolvedForwardAxis))
	{
		UE_LOG(
			LogAnimation,
			Error,
			TEXT("Transfer Hips Motion To Root failed: no valid horizontal forward axis for %s."),
			*GetNameSafe(AnimationSequence));
		ResetStoredData();
		return;
	}

	TArray<FVector> ExtractedOffsets = RawOffsets;
	if (ExtractionMode == EECHipsMotionExtractionMode::ForwardOnly)
	{
		for (int32 KeyIndex = TransferStartKey; KeyIndex <= TransferEndKey; ++KeyIndex)
		{
			const double ForwardDistance =
				FVector::DotProduct(RawOffsets[KeyIndex], ResolvedForwardAxis);
			ExtractedOffsets[KeyIndex] = ResolvedForwardAxis * ForwardDistance;
			if (bExtractVerticalMotion)
			{
				ExtractedOffsets[KeyIndex].Z = RawOffsets[KeyIndex].Z;
			}
		}
	}

	if (ExtractionMode != EECHipsMotionExtractionMode::FullXY)
	{
		SmoothTrajectoryRange(
			ExtractedOffsets,
			TransferStartKey,
			TransferEndKey,
			SmoothingWindowFrames);
		if (bEnforceMonotonicForwardMotion)
		{
			EnforceMonotonicForwardMotion(
				ExtractedOffsets,
				TransferStartKey,
				TransferEndKey,
				ResolvedForwardAxis);
		}
	}

	// Hips height contains two different kinds of motion: the airborne jump arc
	// and the landing/recovery pose (crouching, then straightening the knees).
	// Returning Z only at the end of the sequence incorrectly keeps the capsule
	// airborne during the recovery. Build a separate root-height curve that is
	// guaranteed to reach zero at the authored landing frame. Everything after
	// that frame remains pose motion on Hips.
	TArray<double> ExtractedRootHeightOffsets;
	ExtractedRootHeightOffsets.Init(0.0, NumKeys);
	int32 VerticalLandingKey = TransferEndKey;
	int32 VerticalApexKey = TransferStartKey;
	if (bExtractVerticalMotion)
	{
		VerticalLandingKey = FMath::Clamp(
			FMath::RoundToInt(VerticalLandingNormalizedTime * LastKeyIndex),
			TransferStartKey + 1,
			TransferEndKey);

		const double VerticalStartOffset =
			ExtractedOffsets[TransferStartKey].Z;
		double MaximumHeightAboveStart = 0.0;
		for (int32 KeyIndex = TransferStartKey; KeyIndex <= VerticalLandingKey; ++KeyIndex)
		{
			const double HeightAboveStart =
				ExtractedOffsets[KeyIndex].Z - VerticalStartOffset;
			if (HeightAboveStart > MaximumHeightAboveStart)
			{
				MaximumHeightAboveStart = HeightAboveStart;
				VerticalApexKey = KeyIndex;
			}
		}

		const int32 CorrectionStartKey =
			VerticalApexKey < VerticalLandingKey
				? VerticalApexKey
				: TransferStartKey;
		const double LandingHeightAboveStart =
			ExtractedOffsets[VerticalLandingKey].Z - VerticalStartOffset;
		const double CorrectionKeySpan = static_cast<double>(
			VerticalLandingKey - CorrectionStartKey);

		for (int32 KeyIndex = TransferStartKey + 1;
			 KeyIndex < VerticalLandingKey;
			 ++KeyIndex)
		{
			double HeightAboveStart =
				ExtractedOffsets[KeyIndex].Z - VerticalStartOffset;
			if (KeyIndex > CorrectionStartKey && CorrectionKeySpan > 0.0)
			{
				const double Alpha = static_cast<double>(
					KeyIndex - CorrectionStartKey) / CorrectionKeySpan;
				const double SmoothAlpha = Alpha * Alpha * (3.0 - 2.0 * Alpha);
				HeightAboveStart -= LandingHeightAboveStart * SmoothAlpha;
			}

			// A jump root must not move below its takeoff/landing plane. Negative
			// Hips movement is landing compression and belongs to the pose.
			ExtractedRootHeightOffsets[KeyIndex] =
				FMath::Max(0.0, HeightAboveStart);
		}
	}

	// Motion before the selected range stays entirely on Hips. After the range,
	// hold the extracted root at its landing position and leave recovery sway on Hips.
	const FVector FinalExtractedOffset = ExtractedOffsets[TransferEndKey];
	for (int32 KeyIndex = 0; KeyIndex < TransferStartKey; ++KeyIndex)
	{
		ExtractedOffsets[KeyIndex] = FVector::ZeroVector;
	}
	for (int32 KeyIndex = TransferEndKey + 1; KeyIndex < NumKeys; ++KeyIndex)
	{
		ExtractedOffsets[KeyIndex] = FinalExtractedOffset;
	}

	FBoneTrackKeys NewRootKeys;
	FBoneTrackKeys NewMotionKeys;
	NewRootKeys.Reserve(NumKeys);
	NewMotionKeys.Reserve(NumKeys);

	double MaximumHorizontalDistance = 0.0;
	double MaximumVerticalDistance = 0.0;
	const double FirstRootHeight = OriginalRootTransforms[0].GetLocation().Z;
	const double FixedForwardMappingScale = FVector::DotProduct(
		FixedRootBasis.RotateVector(ResolvedMotionForwardAxis),
		ResolvedForwardAxis);
	if (ExtractionMode == EECHipsMotionExtractionMode::ForwardOnly &&
		FMath::IsNearlyZero(FixedForwardMappingScale))
	{
		UE_LOG(
			LogAnimation,
			Error,
			TEXT("Transfer Hips Motion To Root failed: motion-bone forward axis cannot map to root ForwardAxis in %s."),
			*GetNameSafe(AnimationSequence));
		ResetStoredData();
		return;
	}

	for (int32 KeyIndex = 0; KeyIndex < NumKeys; ++KeyIndex)
	{
		const FTransform& OriginalRoot = OriginalRootTransforms[KeyIndex];
		const FTransform& OriginalMotion = OriginalMotionTransforms[KeyIndex];
		const FVector& ExtractedOffset = ExtractedOffsets[KeyIndex];

		FTransform NewRoot = OriginalRoot;
		NewRoot.AddToTranslation(ExtractedOffset);
		FVector NewRootLocation = NewRoot.GetLocation();
		switch (VerticalRootMotionPolicy)
		{
		case EECRootVerticalMotionPolicy::KeepOnMotionBone:
			NewRootLocation.Z = FirstRootHeight;
			break;

		case EECRootVerticalMotionPolicy::PreserveExistingRoot:
			NewRootLocation.Z = OriginalRoot.GetLocation().Z;
			break;

		case EECRootVerticalMotionPolicy::TransferJumpArcReturnAtLanding:
		default:
			NewRootLocation.Z =
				FirstRootHeight + ExtractedRootHeightOffsets[KeyIndex];
			break;
		}
		NewRoot.SetLocation(NewRootLocation);

		MaximumHorizontalDistance =
			FMath::Max(MaximumHorizontalDistance, ExtractedOffset.Size2D());
		MaximumVerticalDistance = FMath::Max(
			MaximumVerticalDistance,
			FMath::Abs(NewRootLocation.Z - FirstRootHeight));

		// Do not use a component-space relative transform for compensation here.
		// Imported Root bones commonly have a permanent reorientation. Inverting
		// Root X through that rotation can leak forward compensation into Hips Z,
		// making a landed character appear elevated. Remove ownership explicitly
		// from the semantic motion-bone forward/up channels instead.
		FTransform NewMotion = OriginalMotion;
		FVector NewMotionLocation = OriginalMotion.GetLocation();
		if (ExtractionMode == EECHipsMotionExtractionMode::ForwardOnly)
		{
			const double RootForwardDistance = FVector::DotProduct(
				ExtractedOffset,
				ResolvedForwardAxis);
			const double ForwardMappingScale = bUseFirstFrameRootBasis
				? FixedForwardMappingScale
				: FVector::DotProduct(
					OriginalRoot.GetRotation().RotateVector(ResolvedMotionForwardAxis),
					ResolvedForwardAxis);
			if (!FMath::IsNearlyZero(ForwardMappingScale))
			{
				NewMotionLocation -= ResolvedMotionForwardAxis *
					(RootForwardDistance / ForwardMappingScale);
			}
		}
		else
		{
			FVector HorizontalRootOffset = ExtractedOffset;
			HorizontalRootOffset.Z = 0.0;
			FVector MotionBoneCompensation = bUseFirstFrameRootBasis
				? FixedRootBasis.UnrotateVector(HorizontalRootOffset)
				: OriginalRoot.GetRotation().UnrotateVector(HorizontalRootOffset);
			MotionBoneCompensation -= ResolvedMotionUpAxis * FVector::DotProduct(
				MotionBoneCompensation,
				ResolvedMotionUpAxis);
			NewMotionLocation -= MotionBoneCompensation;
		}

		if (bExtractVerticalMotion)
		{
			NewMotionLocation -= ResolvedMotionUpAxis *
				ExtractedRootHeightOffsets[KeyIndex];
		}
		NewMotion.SetLocation(NewMotionLocation);

		NewRootKeys.Add(NewRoot);
		NewMotionKeys.Add(NewMotion);
	}

	if (MaximumHorizontalDistance <= UE_KINDA_SMALL_NUMBER &&
		MaximumVerticalDistance <= UE_KINDA_SMALL_NUMBER)
	{
		UE_LOG(
			LogAnimation,
			Warning,
			TEXT("Transfer Hips Motion To Root: %s has no meaningful extracted '%s' motion."),
			*GetNameSafe(AnimationSequence),
			*MotionBoneName.ToString());
	}

	IAnimationDataController& Controller = AnimationSequence->GetController();
	constexpr bool bShouldTransact = false;
	Controller.OpenBracket(
		LOCTEXT("TransferHipsMotionToRootBracket", "Transfer Hips motion to root"),
		bShouldTransact);

	auto RestoreOriginalState = [&]()
	{
		const bool bTracksRestored =
			RebuildBoneTracks(Controller, OriginalBoneTracks, bShouldTransact);
		AnimationSequence->bEnableRootMotion = bOriginalEnableRootMotion;
		AnimationSequence->bForceRootLock = bOriginalForceRootLock;
		AnimationSequence->RootMotionRootLock = OriginalRootMotionRootLock;
		return bTracksRestored;
	};

	if (OriginalRootTrackIndex != 0)
	{
		// Runtime root-motion extraction requires skeleton root to be track 0.
		// UE 5.5's deprecated InsertBoneTrack can report index 0 while still
		// appending the track, so rebuild the tracks in the required order.
		TArray<FNamedBoneTrackKeys> OrderedTracks = OriginalBoneTracks;

		if (bRootTrackOriginallyExisted)
		{
			FNamedBoneTrackKeys RootTrack =
				MoveTemp(OrderedTracks[OriginalRootTrackIndex]);
			OrderedTracks.RemoveAt(OriginalRootTrackIndex);
			OrderedTracks.Insert(MoveTemp(RootTrack), 0);
		}
		else
		{
			FNamedBoneTrackKeys RootTrack;
			RootTrack.Name = RootBoneName;
			RootTrack.Keys.Reserve(OriginalRootTransforms.Num());
			for (const FTransform& RootTransform : OriginalRootTransforms)
			{
				RootTrack.Keys.Add(RootTransform);
			}
			OrderedTracks.Insert(MoveTemp(RootTrack), 0);
		}

		if (!RebuildBoneTracks(Controller, OrderedTracks, bShouldTransact))
		{
			const bool bRollbackSucceeded = RestoreOriginalState();
			UE_LOG(
				LogAnimation,
				Error,
				TEXT("Transfer Hips Motion To Root failed: could not rebuild bone tracks in %s. Rollback: %s."),
				*GetNameSafe(AnimationSequence),
				bRollbackSucceeded ? TEXT("succeeded") : TEXT("deferred to modifier transaction"));
			Controller.CloseBracket(bShouldTransact);
			ResetStoredData();
			return;
		}
	}

	const bool bRootKeysSet = Controller.SetBoneTrackKeys(
		RootBoneName,
		NewRootKeys.Positions,
		NewRootKeys.Rotations,
		NewRootKeys.Scales,
		bShouldTransact);
	const bool bMotionKeysSet = Controller.SetBoneTrackKeys(
		MotionBoneName,
		NewMotionKeys.Positions,
		NewMotionKeys.Rotations,
		NewMotionKeys.Scales,
		bShouldTransact);

	if (!bRootKeysSet || !bMotionKeysSet)
	{
		const bool bRollbackSucceeded = RestoreOriginalState();
		UE_LOG(
			LogAnimation,
			Error,
			TEXT("Transfer Hips Motion To Root failed: could not write root or motion keys in %s. Rollback: %s."),
			*GetNameSafe(AnimationSequence),
			bRollbackSucceeded ? TEXT("succeeded") : TEXT("deferred to modifier transaction"));
		Controller.CloseBracket(bShouldTransact);
		ResetStoredData();
		return;
	}

	TArray<FName> FinalBoneTrackNames;
	AnimationSequence->GetDataModel()->GetBoneTrackNames(FinalBoneTrackNames);
	if (FinalBoneTrackNames.IsEmpty() || FinalBoneTrackNames[0] != RootBoneName)
	{
		const bool bRollbackSucceeded = RestoreOriginalState();
		UE_LOG(
			LogAnimation,
			Error,
			TEXT("Transfer Hips Motion To Root failed: root track '%s' is not track 0 after modifying %s. Rollback: %s."),
			*RootBoneName.ToString(),
			*GetNameSafe(AnimationSequence),
			bRollbackSucceeded ? TEXT("succeeded") : TEXT("deferred to modifier transaction"));
		Controller.CloseBracket(bShouldTransact);
		ResetStoredData();
		return;
	}

	AnimationSequence->bEnableRootMotion = bEnableRootMotionAfterApply;
	AnimationSequence->bForceRootLock = false;
	// Ref-pose locking is unsafe for imported skeletons whose animation Root has
	// a permanent reorientation. Always lock to the first authored Root key.
	AnimationSequence->RootMotionRootLock = ERootMotionRootLock::AnimFirstFrame;

	Controller.CloseBracket(bShouldTransact);

	// Do not inspect compressed data here. AnimationModifier owns an outer data
	// controller bracket and refreshes compression only after OnApply returns.

	UE_LOG(
		LogAnimation,
		Display,
		TEXT("Transferred '%s' motion to root '%s' in %s (XY %.2f cm, Z %.2f cm, keys %d-%d, landing %d, apex %d)."),
		*MotionBoneName.ToString(),
		*RootBoneName.ToString(),
		*GetNameSafe(AnimationSequence),
		MaximumHorizontalDistance,
		MaximumVerticalDistance,
		TransferStartKey,
		TransferEndKey,
		VerticalLandingKey,
		VerticalApexKey);
}

void UECTransferHipsMotionToRootModifier::OnRevert_Implementation(UAnimSequence* AnimationSequence)
{
	if (!AnimationSequence || !bHasStoredData)
	{
		UE_LOG(
			LogAnimation,
			Warning,
			TEXT("Transfer Hips Motion To Root revert skipped: no stored source data for %s."),
			*GetNameSafe(AnimationSequence));
		return;
	}

	const int32 NumRootKeys = OriginalRootPositions.Num();
	const int32 NumMotionKeys = OriginalMotionPositions.Num();
	const bool bRootDataValid =
		NumRootKeys > 0 &&
		OriginalRootRotations.Num() == NumRootKeys &&
		OriginalRootScales.Num() == NumRootKeys;
	const bool bMotionDataValid =
		NumMotionKeys > 0 &&
		OriginalMotionRotations.Num() == NumMotionKeys &&
		OriginalMotionScales.Num() == NumMotionKeys;

	if (!bRootDataValid || !bMotionDataValid)
	{
		UE_LOG(
			LogAnimation,
			Error,
			TEXT("Transfer Hips Motion To Root revert failed: stored key data is invalid for %s."),
			*GetNameSafe(AnimationSequence));
		return;
	}

	IAnimationDataController& Controller = AnimationSequence->GetController();
	constexpr bool bShouldTransact = false;
	Controller.OpenBracket(
		LOCTEXT("RevertTransferHipsMotionToRootBracket", "Revert Hips XY motion transfer"),
		bShouldTransact);

	if (bRootTrackOriginallyExisted)
	{
		TArray<FName> CurrentBoneTrackNames;
		AnimationSequence->GetDataModel()->GetBoneTrackNames(CurrentBoneTrackNames);
		const int32 CurrentRootTrackIndex =
			CurrentBoneTrackNames.IndexOfByKey(StoredRootBoneName);
		if (CurrentRootTrackIndex != OriginalRootTrackIndex)
		{
			TArray<FNamedBoneTrackKeys> OrderedTracks;
			if (!CaptureBoneTracks(
					AnimationSequence->GetDataModel(),
					OrderedTracks) ||
				!OrderedTracks.IsValidIndex(CurrentRootTrackIndex))
			{
				UE_LOG(
					LogAnimation,
					Error,
					TEXT("Transfer Hips Motion To Root revert failed: could not capture bone tracks in %s."),
					*GetNameSafe(AnimationSequence));
				Controller.CloseBracket(bShouldTransact);
				return;
			}

			FNamedBoneTrackKeys RootTrack =
				MoveTemp(OrderedTracks[CurrentRootTrackIndex]);
			OrderedTracks.RemoveAt(CurrentRootTrackIndex);
			const int32 RestoredRootTrackIndex =
				FMath::Clamp(OriginalRootTrackIndex, 0, OrderedTracks.Num());
			OrderedTracks.Insert(
				MoveTemp(RootTrack),
				RestoredRootTrackIndex);

			if (!RebuildBoneTracks(
					Controller,
					OrderedTracks,
					bShouldTransact))
			{
				UE_LOG(
					LogAnimation,
					Error,
					TEXT("Transfer Hips Motion To Root revert failed: could not restore bone track order in %s."),
					*GetNameSafe(AnimationSequence));
				Controller.CloseBracket(bShouldTransact);
				return;
			}
		}
		Controller.SetBoneTrackKeys(
			StoredRootBoneName,
			OriginalRootPositions,
			OriginalRootRotations,
			OriginalRootScales,
			bShouldTransact);
	}
	else
	{
		Controller.RemoveBoneTrack(StoredRootBoneName, bShouldTransact);
	}
	Controller.SetBoneTrackKeys(
		StoredMotionBoneName,
		OriginalMotionPositions,
		OriginalMotionRotations,
		OriginalMotionScales,
		bShouldTransact);

	AnimationSequence->bEnableRootMotion = bOriginalEnableRootMotion;
	AnimationSequence->bForceRootLock = bOriginalForceRootLock;
	AnimationSequence->RootMotionRootLock = OriginalRootMotionRootLock;

	Controller.CloseBracket(bShouldTransact);
}

int32 UECTransferHipsMotionToRootModifier::GetNativeClassRevision() const
{
	return 10;
}

void UECTransferHipsMotionToRootModifier::ResetStoredData()
{
	bHasStoredData = false;
	bRootTrackOriginallyExisted = false;
	OriginalRootTrackIndex = INDEX_NONE;
	StoredRootBoneName = NAME_None;
	StoredMotionBoneName = NAME_None;
	OriginalRootPositions.Reset();
	OriginalRootRotations.Reset();
	OriginalRootScales.Reset();
	OriginalMotionPositions.Reset();
	OriginalMotionRotations.Reset();
	OriginalMotionScales.Reset();
}

#undef LOCTEXT_NAMESPACE
