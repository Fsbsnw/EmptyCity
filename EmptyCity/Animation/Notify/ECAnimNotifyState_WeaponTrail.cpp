#include "Animation/Notify/ECAnimNotifyState_WeaponTrail.h"

#include "Equipment/Weapon/WeaponTrailComponent.h"
#include "GameFramework/Actor.h"

namespace
{
	UWeaponTrailComponent* ResolveWeaponTrailComponent(USkeletalMeshComponent* MeshComp)
	{
		AActor* OwnerActor = IsValid(MeshComp) ? MeshComp->GetOwner() : nullptr;
		return IsValid(OwnerActor) ? OwnerActor->FindComponentByClass<UWeaponTrailComponent>() : nullptr;
	}
}

UECAnimNotifyState_WeaponTrail::UECAnimNotifyState_WeaponTrail()
{
	bShouldFireInEditor = false;
}

void UECAnimNotifyState_WeaponTrail::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UWeaponTrailComponent* TrailComponent = ResolveWeaponTrailComponent(MeshComp))
	{
		TrailComponent->BeginTrail();
	}
}

void UECAnimNotifyState_WeaponTrail::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (UWeaponTrailComponent* TrailComponent = ResolveWeaponTrailComponent(MeshComp))
	{
		TrailComponent->EndTrail();
	}

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}