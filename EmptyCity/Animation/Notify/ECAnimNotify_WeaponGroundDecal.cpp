#include "Animation/Notify/ECAnimNotify_WeaponGroundDecal.h"

#include "Components/DecalComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"

UECAnimNotify_WeaponGroundDecal::UECAnimNotify_WeaponGroundDecal(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
	NotifyColor = FColor(80, 180, 255, 255);
#endif
}

void UECAnimNotify_WeaponGroundDecal::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp) || !IsValid(DecalMaterial))
	{
		return;
	}

	UWorld* World = MeshComp->GetWorld();
	if (!IsValid(World) || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	USceneComponent* SourceComponent = ResolveSourceComponent(MeshComp);
	FTransform SourceTransform;
	if (!TryGetSourceTransform(SourceComponent, SourceTransform))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[%s] 지면 충격 데칼용 메시 또는 소켓 '%s'을 찾지 못했습니다."),
			*GetNameSafe(MeshComp->GetOwner()),
			*SocketName.ToString());
		return;
	}

	const FVector TraceOrigin =
		SourceTransform.TransformPositionNoScale(SocketLocalOffset);
	const FVector TraceStart = TraceOrigin + FVector::UpVector * TraceUpDistance;
	const FVector TraceEnd = TraceOrigin + FVector::DownVector * TraceDownDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WeaponGroundDecal), false);
	QueryParams.AddIgnoredActor(MeshComp->GetOwner());
	if (IsValid(SourceComponent) && SourceComponent->GetOwner() != MeshComp->GetOwner())
	{
		QueryParams.AddIgnoredActor(SourceComponent->GetOwner());
	}

	FHitResult HitResult;
	const bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		TraceChannel,
		QueryParams);

	if (bDrawDebug)
	{
		DrawDebugLine(
			World,
			TraceStart,
			TraceEnd,
			bHit ? FColor::Green : FColor::Red,
			false,
			2.0f,
			0,
			1.5f);

		if (bHit)
		{
			DrawDebugPoint(World, HitResult.ImpactPoint, 12.0f, FColor::Cyan, false, 2.0f);
		}
	}

	if (!bHit)
	{
		return;
	}

	// Deferred Decal의 투영축이 표면 안쪽을 향하도록 -Normal을 사용합니다.
	const FRotator SurfaceRotation =
		(RotationOffset.Quaternion() * (-HitResult.ImpactNormal).Rotation().Quaternion()).Rotator();

	UDecalComponent* SpawnedDecal = nullptr;
	if (bAttachToHitComponent && HitResult.GetComponent())
	{
		SpawnedDecal = UGameplayStatics::SpawnDecalAttached(
			DecalMaterial,
			DecalSize,
			HitResult.GetComponent(),
			HitResult.BoneName,
			HitResult.ImpactPoint,
			SurfaceRotation,
			EAttachLocation::KeepWorldPosition,
			LifeSpan);
	}
	else
	{
		SpawnedDecal = UGameplayStatics::SpawnDecalAtLocation(
			World,
			DecalMaterial,
			DecalSize,
			HitResult.ImpactPoint,
			SurfaceRotation,
			LifeSpan);
	}

	if (IsValid(SpawnedDecal) && LifeSpan > 0.0f && FadeOutDuration > 0.0f)
	{
		const float ClampedFadeDuration = FMath::Min(FadeOutDuration, LifeSpan);
		SpawnedDecal->SetFadeOut(
			LifeSpan - ClampedFadeDuration,
			ClampedFadeDuration,
			false);
	}
}

FString UECAnimNotify_WeaponGroundDecal::GetNotifyName_Implementation() const
{
	return SocketName.IsNone()
		? TEXT("Ground Impact Decal")
		: FString::Printf(TEXT("Ground Impact Decal (%s)"), *SocketName.ToString());
}

USceneComponent* UECAnimNotify_WeaponGroundDecal::ResolveSourceComponent(
	USkeletalMeshComponent* AnimationMesh) const
{
	if (!IsValid(AnimationMesh))
	{
		return nullptr;
	}

	if (Source != EECWeaponDecalSource::AnimationMesh)
	{
		if (USceneComponent* WeaponComponent = FindAttachedWeaponComponent(AnimationMesh))
		{
			return WeaponComponent;
		}

		if (Source == EECWeaponDecalSource::AttachedWeapon)
		{
			return nullptr;
		}
	}

	return SocketName.IsNone() || AnimationMesh->DoesSocketExist(SocketName)
		? AnimationMesh
		: nullptr;
}

USceneComponent* UECAnimNotify_WeaponGroundDecal::FindAttachedWeaponComponent(
	USkeletalMeshComponent* AnimationMesh) const
{
	AActor* OwnerActor = IsValid(AnimationMesh) ? AnimationMesh->GetOwner() : nullptr;
	if (!IsValid(OwnerActor))
	{
		return nullptr;
	}

	// BP 내부에서 캐릭터 Skeletal Mesh에 직접 부착한 StaticMeshComponent도 찾습니다.
	TArray<USceneComponent*> PendingComponents = AnimationMesh->GetAttachChildren();
	while (!PendingComponents.IsEmpty())
	{
		USceneComponent* AttachedComponent = PendingComponents.Pop(EAllowShrinking::No);
		if (!IsValid(AttachedComponent))
		{
			continue;
		}

		PendingComponents.Append(AttachedComponent->GetAttachChildren());

		if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(AttachedComponent))
		{
			if (SocketName.IsNone()
				|| MeshComponent->DoesSocketExist(SocketName)
				|| MeshComponent->GetAttachSocketName() == SocketName
				|| MeshComponent->GetFName() == SocketName)
			{
				return MeshComponent;
			}
		}
	}

	TArray<AActor*> AttachedActors;
	OwnerActor->GetAttachedActors(AttachedActors, true, true);

	for (AActor* AttachedActor : AttachedActors)
	{
		if (!IsValid(AttachedActor))
		{
			continue;
		}

		TInlineComponentArray<UMeshComponent*> MeshComponents(AttachedActor);
		for (UMeshComponent* MeshComponent : MeshComponents)
		{
			if (IsValid(MeshComponent)
				&& (SocketName.IsNone()
					|| MeshComponent->DoesSocketExist(SocketName)
					|| MeshComponent->GetAttachSocketName() == SocketName
					|| MeshComponent->GetFName() == SocketName))
			{
				return MeshComponent;
			}
		}
	}

	return nullptr;
}

bool UECAnimNotify_WeaponGroundDecal::TryGetSourceTransform(
	USceneComponent* SourceComponent,
	FTransform& OutTransform) const
{
	if (!IsValid(SourceComponent))
	{
		return false;
	}

	if (SocketName.IsNone())
	{
		OutTransform = SourceComponent->GetComponentTransform();
		return true;
	}

	if (!SourceComponent->DoesSocketExist(SocketName))
	{
		// StaticMeshComponent가 캐릭터의 해당 소켓에 붙어 있거나 컴포넌트 이름으로
		// 선택된 경우에는 무기 컴포넌트 원점을 기준으로 로컬 오프셋을 적용합니다.
		if (Source != EECWeaponDecalSource::AnimationMesh
			&& (SourceComponent->GetAttachSocketName() == SocketName
				|| SourceComponent->GetFName() == SocketName))
		{
			OutTransform = SourceComponent->GetComponentTransform();
			return true;
		}

		return false;
	}

	OutTransform = SourceComponent->GetSocketTransform(SocketName, RTS_World);
	return true;
}
