#include "ECGameplayAbility_Death.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "Character/Player/ECPlayer.h"
#include "Component/Attribute/ECHealthComponent.h"
#include "ECGameplayTags.h"
#include "Engine/GameInstance.h"
#include "Subsystem/ECGameFlowSubsystem.h"
#include "TimerManager.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Engine/World.h"
#include "Subsystem/ECGameplayMessageSubsystem.h"

UECGameplayAbility_Death::UECGameplayAbility_Death()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	CancelAbilitiesWithTag.AddTag(ECGameplayTags::Ability);
	BlockAbilitiesWithTag.AddTag(ECGameplayTags::Ability);
	ActivationOwnedTags.AddTag(ECGameplayTags::Status_Death_Dying);
	// 공격/가드/파밍도 Exclusive_Blocking이므로 사망까지 같은 그룹이면
	// 해당 동작 중 체력이 0이 되었을 때 사망 어빌리티가 활성화되지 않습니다.
	// 사망은 항상 발동한 뒤 ActivateAbility에서 다른 어빌리티를 직접 취소합니다.
	ActivationGroup = EAbilityActivationGroup::Independent;
	
	// 고정되는 값이므로 CDO일 때 진행합니다.
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = ECGameplayTags::GameplayEvent_Death;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UECGameplayAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 발동 중인 다른 어빌리티들을 모두 취소합니다.
	UECAbilitySystemComponent* ASC = CastChecked<UECAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	ASC->CancelAbilities(nullptr, nullptr, this);
	ASC->ClearAbilityInput();
	
	// Death 어빌리티가 취소되지 않게 잠금 상태로 전환합니다.
	SetCanBeCanceled(false);
	
	// 사망 상태로 전환합니다.
	StartDeath();
	BroadcastPlayerDeathMessage(ActorInfo, TriggerEventData);
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UECGameplayAbility_Death::BroadcastPlayerDeathMessage(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayEventData* TriggerEventData) const
{
	AECPlayer* Player = ActorInfo
		? Cast<AECPlayer>(ActorInfo->AvatarActor.Get())
		: nullptr;
	if (!Player)
	{
		return;
	}

	FECPlayerDeathMessage Message;
	Message.PlayerActor = Player;
	Message.PlayerController = ActorInfo->PlayerController.Get();
	Message.DamageInstigator = TriggerEventData
		? const_cast<AActor*>(TriggerEventData->Instigator.Get())
		: nullptr;

	UECGameplayMessageSubsystem::Get(Player).BroadcastMessage(
		TAG_Message_Player_Death,
		Message);
}

void UECGameplayAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	check(ActorInfo);

	AECPlayer* Player = Cast<AECPlayer>(ActorInfo->AvatarActor.Get());
	if (!Player)
	{
		FinishDeath();
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	// Super 호출 뒤에는 사망 어빌리티의 차단 태그와 Exclusive 그룹이 모두 해제됩니다.
	// 그 다음 GameFlow가 플레이어와 상시 어빌리티를 안전하게 복구합니다.
	if (Player)
	{
		if (UGameInstance* GameInstance = Player->GetGameInstance())
		{
			if (UECGameFlowSubsystem* GameFlow = GameInstance->GetSubsystem<UECGameFlowSubsystem>())
			{
				GameFlow->CompletePlayerDeathFlow(Player);
			}
		}
	}
}

void UECGameplayAbility_Death::StartDeath()
{
	if (UECHealthComponent* HealthComponent = UECHealthComponent::FindHealthComponent(GetAvatarActorFromActorInfo()))
	{
		if (HealthComponent->GetDeathState() == EDeathState::NotDead)
		{
			HealthComponent->StartDeath();
		}
	}

	StartDeathEndTimer();
}

void UECGameplayAbility_Death::FinishDeath()
{
	if (UECHealthComponent* HealthComponent = UECHealthComponent::FindHealthComponent(GetAvatarActorFromActorInfo()))
	{
		if (HealthComponent->GetDeathState() == EDeathState::DeathStarted)
		{
			HealthComponent->FinishDeath();
		}
	}
}

void UECGameplayAbility_Death::StartDeathEndTimer()
{
	GetWorld()->GetTimerManager().SetTimer(
		DeathEndTimerHandle,
		this,
		&ThisClass::K2_EndAbility,
		DeathEndDelay,
		false
	);
}
