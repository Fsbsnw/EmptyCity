#include "AbilitySystem/Ability/Player/ECGameplayAbility_Farm.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Actor/Farming/ECFarmingResourceActor.h"
#include "Character/Player/ECPlayer.h"
#include "Data/Farming/ECFarmingTableRow.h"
#include "Equipment/ECEquipmentInstance.h"
#include "Equipment/ECEquipmentManagerComponent.h"
#include "Equipment/Weapon/ECWeaponInstance.h"

UECGameplayAbility_Farm::UECGameplayAbility_Farm()
{
	// 파밍 Ability의 인스턴스, 실행 정책 및 동시 실행 정책을 설정합니다.
	{
		InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
		ActivationPolicy = EAbilityActivationPolicy::ByGameplayEvent;
		ActivationGroup = EAbilityActivationGroup::Exclusive_Blocking;
	}
}

void UECGameplayAbility_Farm::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 상호작용 이벤트에서 파밍 대상과 파밍 데이터를 가져옵니다.
	FarmingTarget = Cast<AECFarmingResourceActor>(const_cast<AActor*>(ToRawPtr(TriggerEventData->Target)));
	const FECFarmingTableRow& FarmingData = FarmingTarget->GetFarmingData();

	// 플레이어의 장비 관리자와 현재 무기를 보관합니다.
	AECPlayer* Player = Cast<AECPlayer>(GetAvatarActorFromActorInfo());
	EquipmentManager = Player->FindComponentByClass<UECEquipmentManagerComponent>();
	EquippedWeapon = Player->GetEquippedWeapon();

	// 파밍 도구가 보이도록 현재 무기 Actor를 숨깁니다.
	for (AActor* WeaponActor : EquippedWeapon->GetSpawnedActors())
	{
		WeaponActor->SetActorHiddenInGame(true);
	}

	// 파밍 데이터에 지정된 도구를 임시 장착합니다.
	FarmingTool = EquipmentManager->EquipItem(FarmingData.ToolEquipmentDefinition);

	// 파밍 몽타주를 재생합니다.
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, FarmingData.FarmingMontage.LoadSynchronous());
	MontageTask->ReadyForActivation();

	// 데이터에 지정된 홀딩 시간이 끝나면 파밍을 완료합니다.
	UAbilityTask_WaitDelay* FarmingTimer = UAbilityTask_WaitDelay::WaitDelay(this, FarmingData.InteractionDuration);
	FarmingTimer->OnFinish.AddDynamic(this, &ThisClass::HandleFarmingCompleted);
	FarmingTimer->ReadyForActivation();
}

void UECGameplayAbility_Farm::HandleFarmingCompleted()
{
	// 아이템을 드롭하고 파밍 대상을 영구 제거합니다.
	FarmingTarget->CompleteFarming();
	K2_EndAbility();
}

void UECGameplayAbility_Farm::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 파밍 중 임시 장착한 도구를 해제합니다.
	EquipmentManager->UnequipItem(FarmingTool);

	// 숨겨 두었던 기존 무기 Actor를 다시 표시합니다.
	for (AActor* WeaponActor : EquippedWeapon->GetSpawnedActors())
	{
		WeaponActor->SetActorHiddenInGame(false);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
