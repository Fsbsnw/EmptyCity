#include "Animation/Notify/ECAnimNotify_CameraShake.h"

#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UECAnimNotify_CameraShake::UECAnimNotify_CameraShake(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
	NotifyColor = FColor(255, 170, 40, 255);
#endif
}

void UECAnimNotify_CameraShake::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	const APawn* OwningPawn = IsValid(MeshComp) ? Cast<APawn>(MeshComp->GetOwner()) : nullptr;
	APlayerController* PlayerController =
		IsValid(OwningPawn) ? Cast<APlayerController>(OwningPawn->GetController()) : nullptr;

	if (!IsValid(PlayerController)
		|| !PlayerController->IsLocalController()
		|| !IsValid(PlayerController->PlayerCameraManager)
		|| !ShakeClass)
	{
		return;
	}

	PlayerController->PlayerCameraManager->StartCameraShake(ShakeClass, Scale);
}
