#include "ECGameplayAbility_Interact.h"
#include "AbilitySystemComponent.h"
#include "ECGameplayTags.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionStatics.h"
#include "Interaction/Task/AbilityTask_GrantNearbyInteraction.h"
#include "NativeGameplayTags.h"
#include "Interaction/InteractionQuery.h"
#include "Interaction/Task/AbilityTask_WaitForInteractableTargets_SingleLineTrace.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Character/Player/Controller/ECPlayerController.h"

class UUserWidget;

// 상호작용을 발동할 때 사용하는 게임플레이 태그들입니다.
// "Ability.Interaction.Activate"는 상호작용 어빌리티를 실행시키는 이벤트의 식별자 역할을 합니다.
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Interaction_Activate, "Ability.Interaction.Activate");
UE_DEFINE_GAMEPLAY_TAG(TAG_INTERACTION_DURATION_MESSAGE, "Ability.Interaction.Duration.Message");

UECGameplayAbility_Interact::UECGameplayAbility_Interact()
{
	// OnSpawn: 캐릭터가 스폰되면 자동으로 이 어빌리티가 발동됩니다.
	ActivationPolicy = EAbilityActivationPolicy::OnSpawn;
	
	// InstancedPerActor: 어빌리티 인스턴스를 액터마다 하나씩 두어 상태(CurrentOptions 등)를 유지합니다.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
}

void UECGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// 1. 플레이어 주변 반경으로 Overlap을 시작합니다.
	StartNearbyAbilityGranting();
	
	// 2. 플레이어 정면으로 LineTrace을 시작합니다.
	StartLookForInteractables();
}

void UECGameplayAbility_Interact::InputStarted()
{
	Super::InputStarted();

	TriggerInteraction();
}

void UECGameplayAbility_Interact::StartNearbyAbilityGranting()
{
	UAbilityTask_GrantNearbyInteraction* Task = UAbilityTask_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractors(this, InteractionScanRange, InteractionScanRate, true);
	Task->ReadyForActivation();
}

void UECGameplayAbility_Interact::StartLookForInteractables()
{
	FInteractionQuery Query;
	Query.RequestingAvatar = GetAvatarActorFromActorInfo();
	Query.RequestingController = Cast<AController>(GetAvatarActorFromActorInfo());
	
	UAbilityTask_WaitForInteractableTargets_SingleLineTrace* Task = UAbilityTask_WaitForInteractableTargets_SingleLineTrace::WaitForInteractableTargets_SingleLineTrace(this, Query, FName(TEXT("Interactable_BlockDynamic")), MakeTargetLocationInfoFromOwnerActor(), InteractionScanRange, InteractionScanRate, true);
	Task->InteractableObjectsChanged.AddDynamic(this, &ThisClass::UpdateInteractions);
	Task->ReadyForActivation();
}

void UECGameplayAbility_Interact::UpdateInteractions(const TArray<FInteractionOption>& InteractiveOptions)
{
	// 최신 선택지를 저장해 둡니다. TriggerInteraction()에서 이 목록을 사용합니다.
	CurrentOptions = InteractiveOptions;

	
	if (AECPlayerController* PC = GetECPlayerControllerFromActorInfo())
	{
		// 인디케이터 매니저 대신, PC에 만들어둘 블루프린트 이벤트를 호출합니다.(임시)
		// 선택지가 1개 이상 있으면 true, 없으면 false를 넘겨 UI를 켜고 끄게 만듭니다.
		bool bCanInteract = CurrentOptions.Num() > 0;
		PC->K2_UpdateInteractionUI(bCanInteract, CurrentOptions);
		
		// @TODO : InteractiveOptions를 받아서 UI를 화면에 표시
        
		/*if (ULyraIndicatorManagerComponent* IndicatorManager = ULyraIndicatorManagerComponent::GetComponent(PC))
		{
			for (UIndicatorDescriptor* Indicator : Indicators)
			{
				IndicatorManager->RemoveIndicator(Indicator);
			}
			Indicators.Reset();

			for (const FInteractionOption& InteractionOption : InteractiveOptions)
			{
				AActor* InteractableTargetActor = UInteractionStatics::GetActorFromInteractableTarget(InteractionOption.InteractableTarget);

				TSoftClassPtr<UUserWidget> InteractionWidgetClass = 
					InteractionOption.InteractionWidgetClass.IsNull() ? DefaultInteractionWidgetClass : InteractionOption.InteractionWidgetClass;

				UIndicatorDescriptor* Indicator = NewObject<UIndicatorDescriptor>();
				Indicator->SetDataObject(InteractableTargetActor);
				Indicator->SetSceneComponent(InteractableTargetActor->GetRootComponent());
				Indicator->SetIndicatorClass(InteractionWidgetClass);
				IndicatorManager->AddIndicator(Indicator);

				Indicators.Add(Indicator);
			}
		}
		else
		{
			//TODO This should probably be a noisy warning.  Why are we updating interactions on a PC that can never do anything with them?
		}*/
	}
}

void UECGameplayAbility_Interact::TriggerInteraction(float TimeWaited)
{
	UE_LOG(LogTemp, Warning, TEXT("상호작용 키 눌림"));

	// 사용할 수 있는 선택지가 없으면 아무것도 하지 않습니다.
	if (CurrentOptions.Num() == 0)
	{
		return;
	}

	// 일단 첫 번째 선택지를 사용합니다(여러 선택지 중 고르는 로직은 추후 확장 가능).
	const FInteractionOption& InteractionOption = CurrentOptions[0];

	// 상호작용을 시작한 주체(플레이어)
	AActor* Instigator = GetAvatarActorFromActorInfo();	
	
	// 상호작용 대상 액터
	AActor* InteractableTargetActor = UInteractionStatics::GetActorFromInteractableTarget(InteractionOption.InteractableTarget);

	// 상호작용 어빌리티로 전달할 이벤트 데이터(Payload)를 구성합니다.
	FGameplayEventData Payload;
	Payload.EventTag = TAG_Ability_Interaction_Activate;
	Payload.Instigator = Instigator;
	Payload.Target = InteractableTargetActor;

	// 대상이 이벤트 데이터를 자기 상황에 맞게 수정할 기회를 줍니다.
	// 예) 벽에 달린 버튼은 자기 자신이 아니라 "문 액터"를 Target으로 바꿔 지정할 수 있습니다.
	InteractionOption.InteractableTarget->CustomizeInteractionEventData(TAG_Ability_Interaction_Activate, Payload);

	// 위에서 (대상이 수정했을 수도 있는) Payload.Target을 꺼내, 상호작용의 '아바타'로 사용합니다.
	AActor* TargetActor = const_cast<AActor*>(ToRawPtr(Payload.Target));

	// 상호작용 어빌리티 실행에 필요한 액터 정보를 구성합니다.
	// (소유자 = 상호작용 대상 액터, 아바타 = 위에서 꺼낸 TargetActor)
	FGameplayAbilityActorInfo ActorInfo;
	ActorInfo.InitFromActor(InteractableTargetActor, TargetActor, InteractionOption.TargetAbilitySystem);

	// 이벤트 태그를 통해 대상 쪽 어빌리티 시스템에서 상호작용 어빌리티를 실제로 발동시킵니다.
	InteractionOption.TargetAbilitySystem->TriggerAbilityFromGameplayEvent(
		InteractionOption.TargetInteractionAbilityHandle,
		&ActorInfo,
		TAG_Ability_Interaction_Activate,
		&Payload,
		*InteractionOption.TargetAbilitySystem
	);
}

