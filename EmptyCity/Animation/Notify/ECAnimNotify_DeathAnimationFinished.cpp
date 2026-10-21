#include "Animation/Notify/ECAnimNotify_DeathAnimationFinished.h"

#include "Character/Enemy/ECEnemyCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"

UECAnimNotify_DeathAnimationFinished::UECAnimNotify_DeathAnimationFinished(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
	NotifyColor = FColor(145, 85, 255, 255);
#endif
}

void UECAnimNotify_DeathAnimationFinished::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AECEnemyCharacterBase* EnemyCharacter =
		IsValid(MeshComp) ? Cast<AECEnemyCharacterBase>(MeshComp->GetOwner()) : nullptr;

	if (IsValid(EnemyCharacter))
	{
		EnemyCharacter->HandleDeathAnimationFinished();
	}
}
