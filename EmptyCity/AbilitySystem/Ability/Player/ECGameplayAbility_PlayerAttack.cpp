#include "AbilitySystem/Ability/Player/ECGameplayAbility_PlayerAttack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "ECGameplayTags.h"
#include "Equipment/Weapon/ECWeaponInstance.h"

UECGameplayAbility_PlayerAttack::UECGameplayAbility_PlayerAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationPolicy = EAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = EAbilityActivationGroup::Exclusive_Blocking;

	FGameplayTagContainer AssetTags = GetAssetTags();
	AssetTags.AddTag(ECGameplayTags::Ability_Type_Action_Attack);
	SetAssetTags(AssetTags);
	ActivationOwnedTags.AddTag(ECGameplayTags::Status_Attacking);
}

void UECGameplayAbility_PlayerAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	FGameplayTag AttackTag = GetAssetTags().First();
	
	UECWeaponInstance* WeaponInstance = Cast<UECWeaponInstance>(GetAssociatedEquipment());
	const FECWeaponAttackDefinition* AttackDefinition = IsValid(WeaponInstance) ? WeaponInstance->SetActiveAttack(AttackTag) : nullptr;

	if (!AttackTag.IsValid() || !AttackDefinition || !IsValid(AttackDefinition->AttackMontage))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 공격 태그 %s에 대응하는 무기 공격 정의 또는 몽타주가 없습니다."), *GetNameSafe(GetAvatarActorFromActorInfo()), *AttackTag.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			AttackDefinition->AttackMontage);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageTask->ReadyForActivation();
}

void UECGameplayAbility_PlayerAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UECWeaponInstance* WeaponInstance = Cast<UECWeaponInstance>(GetAssociatedEquipment()))
	{
		WeaponInstance->ClearActiveAttack();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
