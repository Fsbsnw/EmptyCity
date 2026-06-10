#include "InteractionStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/OverlapResult.h"
#include "IInteractableTarget.h"
#include "UObject/ScriptInterface.h"

AActor* UInteractionStatics::GetActorFromInteractableTarget(TScriptInterface<IInteractableTarget> InteractableTarget)
{
	if (UObject* Object = InteractableTarget.GetObject())
	{
		// 대상이 액터인 경우: 그 액터를 그대로 반환합니다.
		if (AActor* Actor = Cast<AActor>(Object))
		{
			return Actor;
		}
		// 대상이 컴포넌트인 경우: 그 컴포넌트를 소유한 액터를 반환합니다.
		else if (UActorComponent* ActorComponent = Cast<UActorComponent>(Object))
		{
			return ActorComponent->GetOwner();
		}
		else
		{
			// 액터도 컴포넌트도 아닌 경우는 예상하지 못한 상황이므로 미구현으로 표시합니다.
			unimplemented();
		}
	}

	return nullptr;
}

void UInteractionStatics::GetInteractableTargetsFromActor(AActor* Actor, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets)
{
	// 액터 자체가 상호작용 인터페이스를 구현하고 있다면 그것을 추가합니다.
	TScriptInterface<IInteractableTarget> InteractableActor(Actor);
	if (InteractableActor)
	{
		OutInteractableTargets.Add(InteractableActor);
	}

	// 액터 자체는 상호작용 불가능하더라도, 상호작용 인터페이스를 구현한 컴포넌트를 가지고 있을 수 있습니다.
	TArray<UActorComponent*> InteractableComponents = Actor ? Actor->GetComponentsByInterface(UInteractableTarget::StaticClass()) : TArray<UActorComponent*>();
	for (UActorComponent* InteractableComponent : InteractableComponents)
	{
		OutInteractableTargets.Add(TScriptInterface<IInteractableTarget>(InteractableComponent));
	}
}

void UInteractionStatics::AppendInteractableTargetsFromOverlapResults(const TArray<FOverlapResult>& OverlapResults, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets)
{
	for (const FOverlapResult& Overlap : OverlapResults)
	{
		// 겹친 액터가 상호작용 가능하면 추가합니다. AddUnique로 중복을 막습니다.
		TScriptInterface<IInteractableTarget> InteractableActor(Overlap.GetActor());
		if (InteractableActor)
		{
			OutInteractableTargets.AddUnique(InteractableActor);
		}

		// 겹친 컴포넌트가 상호작용 가능하면 추가합니다.
		TScriptInterface<IInteractableTarget> InteractableComponent(Overlap.GetComponent());
		if (InteractableComponent)
		{
			OutInteractableTargets.AddUnique(InteractableComponent);
		}
	}
}

void UInteractionStatics::AppendInteractableTargetsFromHitResult(const FHitResult& HitResult, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets)
{
	// 충돌한 액터가 상호작용 가능하면 추가합니다.
	TScriptInterface<IInteractableTarget> InteractableActor(HitResult.GetActor());
	if (InteractableActor)
	{
		OutInteractableTargets.AddUnique(InteractableActor);
	}

	// 충돌한 컴포넌트가 상호작용 가능하면 추가합니다.
	TScriptInterface<IInteractableTarget> InteractableComponent(HitResult.GetComponent());
	if (InteractableComponent)
	{
		OutInteractableTargets.AddUnique(InteractableComponent);
	}
}
