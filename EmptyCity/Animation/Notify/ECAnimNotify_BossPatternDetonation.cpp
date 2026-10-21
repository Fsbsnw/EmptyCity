#include "Animation/Notify/ECAnimNotify_BossPatternDetonation.h"

#include "Character/Enemy/Component/BossPatternComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UECAnimNotify_BossPatternDetonation::UECAnimNotify_BossPatternDetonation(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
	NotifyColor = FColor(255, 100, 50, 255);
#endif
}

void UECAnimNotify_BossPatternDetonation::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = IsValid(MeshComp) ? MeshComp->GetOwner() : nullptr;
	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}

	if (UECBossPatternComponent* PatternComponent =
		Owner->FindComponentByClass<UECBossPatternComponent>())
	{
		PatternComponent->StartSequentialDetonation();
	}
}

FString UECAnimNotify_BossPatternDetonation::GetNotifyName_Implementation() const
{
	return TEXT("Boss Pattern Start Detonation");
}
