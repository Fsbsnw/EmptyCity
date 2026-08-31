#include "Animation/Notify/ECAnimNotifyState_GameplayTagWindow.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
	UAbilitySystemComponent* ResolveAbilitySystemComponent(const USkeletalMeshComponent* MeshComp)
	{
		AActor* Owner = IsValid(MeshComp) ? MeshComp->GetOwner() : nullptr;
		return IsValid(Owner)
			? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner)
			: nullptr;
	}
}

UECAnimNotifyState_GameplayTagWindow::UECAnimNotifyState_GameplayTagWindow(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bIsNativeBranchingPoint = true;

#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
	NotifyColor = FColor(80, 180, 255, 255);
#endif
}

void UECAnimNotifyState_GameplayTagWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!WindowTag.IsValid())
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = ResolveAbilitySystemComponent(MeshComp))
	{
		ASC->SetLooseGameplayTagCount(WindowTag, 1);
	}
}

void UECAnimNotifyState_GameplayTagWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (WindowTag.IsValid())
	{
		if (UAbilitySystemComponent* ASC = ResolveAbilitySystemComponent(MeshComp))
		{
			ASC->SetLooseGameplayTagCount(WindowTag, 0);
		}
	}

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
