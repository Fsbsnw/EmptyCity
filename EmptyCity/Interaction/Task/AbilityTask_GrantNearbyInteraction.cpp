#include "AbilityTask_GrantNearbyInteraction.h"
#include "AbilitySystemComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionOption.h"
#include "Interaction/InteractionQuery.h"
#include "Interaction/InteractionStatics.h"
#include "Define/ECCollisionChannels.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

UAbilityTask_GrantNearbyInteraction* UAbilityTask_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractors(UGameplayAbility* OwningAbility, float InteractionScanRange, float InteractionScanRate, bool bShowDebug)
{
	// 태스크 인스턴스를 생성하고 입력 파라미터를 채워 반환합니다.
	UAbilityTask_GrantNearbyInteraction* MyObj = NewAbilityTask<UAbilityTask_GrantNearbyInteraction>(OwningAbility);
	MyObj->InteractionScanRange = InteractionScanRange;
	MyObj->InteractionScanRate = InteractionScanRate;
	MyObj->bShowDebug = bShowDebug;
	return MyObj;
}

void UAbilityTask_GrantNearbyInteraction::Activate()
{
	// 아바타(캐릭터)가 준비될 때까지 기다리도록 설정합니다.
	SetWaitingOnAvatar();

	// InteractionScanRate 주기로 QueryInteractables를 반복 호출하는 타이머를 켭니다.
	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(QueryTimerHandle, this, &ThisClass::QueryInteractables, InteractionScanRate, true);
}

void UAbilityTask_GrantNearbyInteraction::OnDestroy(bool AbilityEnded)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(QueryTimerHandle);
	}

	Super::OnDestroy(AbilityEnded);
}

void UAbilityTask_GrantNearbyInteraction::QueryInteractables()
{
	UWorld* World = GetWorld();
	AActor* ActorOwner = GetAvatarActor();
	
	if (World && ActorOwner)
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(UAbilityTask_GrantNearbyInteraction), false);

		// 플레이어 위치를 중심으로 InteractionScanRange 반경의 구체 겹침 검사를 수행합니다.
		// EC_TraceChannel_Interaction 채널을 사용해 상호작용용 콜리전만 골라냅니다.
		TArray<FOverlapResult> OverlapResults;
		World->OverlapMultiByChannel(OUT OverlapResults, ActorOwner->GetActorLocation(), FQuat::Identity, EC_TraceChannel_Interaction, FCollisionShape::MakeSphere(InteractionScanRange), Params);

#if ENABLE_DRAW_DEBUG
		if (bShowDebug)
		{
			DrawDebugSphere(World, ActorOwner->GetActorLocation(), InteractionScanRange, 16, FColor::Cyan, false, InteractionScanRate);
		}
#endif

		if (OverlapResults.Num() > 0)
		{
			// 겹친 결과에서 상호작용 가능한 대상들만 추출합니다.
			TArray<TScriptInterface<IInteractableTarget>> InteractableTargets;
			UInteractionStatics::AppendInteractableTargetsFromOverlapResults(OverlapResults, OUT InteractableTargets);

			// "누가 요청하는지"를 담은 쿼리를 만듭니다(요청자 = 이 캐릭터).
			FInteractionQuery InteractionQuery;
			InteractionQuery.RequestingAvatar = ActorOwner;
			InteractionQuery.RequestingController = Cast<AController>(ActorOwner->GetOwner());

			// 추출한 상호작용 대상에게 상호작용 옵션을 수집해 달라고 요청해, Options 배열에 모읍니다.
			TArray<FInteractionOption> Options;
			for (TScriptInterface<IInteractableTarget>& InteractiveTarget : InteractableTargets)
			{
				FInteractionOptionBuilder InteractionBuilder(InteractiveTarget, OUT Options);
				InteractiveTarget->GatherInteractionOptions(InteractionQuery, InteractionBuilder);
			}

			// 수집된 옵션 중 플레이어에게 어빌리티 부여가 필요한 것이 있는지 확인합니다.
			for (FInteractionOption& Option : Options)
			{
				if (Option.InteractionAbilityToGrant)
				{
					// 어빌리티를 부여해야 실제로 상호작용을 실행할 수 있습니다.
					FObjectKey ObjectKey(Option.InteractionAbilityToGrant);

					// 캐시에 없는 어빌리티만 새로 부여합니다(중복 부여 방지).
					if (!InteractionAbilityCache.Find(ObjectKey))
					{
						FGameplayAbilitySpec Spec(Option.InteractionAbilityToGrant, 1, INDEX_NONE, this);
						FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(Spec);
						InteractionAbilityCache.Add(ObjectKey, Handle);
					}
				}
			}
		}
	}
}

