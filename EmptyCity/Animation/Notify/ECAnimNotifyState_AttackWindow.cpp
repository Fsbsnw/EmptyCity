#include "Animation/Notify/ECAnimNotifyState_AttackWindow.h"
#include "Character/Player/ECPlayer.h"
#include "Equipment/Weapon/ECWeaponInstance.h"

namespace
{
	UECWeaponInstance* ResolveEquippedWeapon(USkeletalMeshComponent* MeshComp)
	{
		const AECPlayer* Player = IsValid(MeshComp) ? Cast<AECPlayer>(MeshComp->GetOwner()) : nullptr;

		return IsValid(Player) ? Player->GetEquippedWeapon() : nullptr;
	}
}

UECAnimNotifyState_AttackWindow::UECAnimNotifyState_AttackWindow(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bIsNativeBranchingPoint = true;

#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
	NotifyColor = FColor(255, 80, 80, 255);
#endif
}

void UECAnimNotifyState_AttackWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UECWeaponInstance* WeaponInstance = ResolveEquippedWeapon(MeshComp))
	{
		WeaponInstance->BeginTrace();
	}
}

void UECAnimNotifyState_AttackWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (UECWeaponInstance* WeaponInstance = ResolveEquippedWeapon(MeshComp))
	{
		WeaponInstance->EndTrace();
	}

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
