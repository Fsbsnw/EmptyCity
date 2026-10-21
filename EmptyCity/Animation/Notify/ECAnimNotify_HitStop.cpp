#include "Animation/Notify/ECAnimNotify_HitStop.h"

#include "Character/Enemy/Component/HitReactionComponent.h"
#include "Components/SkeletalMeshComponent.h"

UECAnimNotify_HitStop::UECAnimNotify_HitStop(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
	NotifyColor = FColor(255, 80, 40, 255);
#endif
}

void UECAnimNotify_HitStop::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = IsValid(MeshComp) ? MeshComp->GetOwner() : nullptr;
	if (!IsValid(Owner))
	{
		return;
	}

	if (UHitReactionComponent* HitReactionComponent =
		Owner->FindComponentByClass<UHitReactionComponent>())
	{
		HitReactionComponent->PlayQueuedHitStop();
	}
}
