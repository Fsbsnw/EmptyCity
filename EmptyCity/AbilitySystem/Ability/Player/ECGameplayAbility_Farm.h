#pragma once

#include "AbilitySystem/Ability/ECGameplayAbility.h"
#include "ECGameplayAbility_Farm.generated.h"

class AECFarmingResourceActor;
class UECEquipmentInstance;
class UECEquipmentManagerComponent;
class UECWeaponInstance;

/** 파밍 도구와 몽타주를 재생하고 지정된 홀딩 시간이 끝나면 자원을 드롭합니다. */
UCLASS()
class EMPTYCITY_API UECGameplayAbility_Farm : public UECGameplayAbility
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// GameplayAbility Interface
// ─────────────────────────────────────────────────────────────
public:
	/** 파밍 Ability의 실행 정책을 설정합니다. */
	UECGameplayAbility_Farm();

	/** 파밍 대상의 도구, 몽타주 및 완료 타이머를 시작합니다. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** 임시 도구를 해제하고 기존 무기를 다시 표시합니다. */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;


// ─────────────────────────────────────────────────────────────
// Farming
// ─────────────────────────────────────────────────────────────
private:
	/** 홀딩 시간이 끝나면 파밍 대상을 완료 처리합니다. */
	UFUNCTION()
	void HandleFarmingCompleted();


// ─────────────────────────────────────────────────────────────
// Cached Variables
// ─────────────────────────────────────────────────────────────
private:
	/** 현재 파밍 중인 대상을 보관합니다. */
	UPROPERTY()
	TObjectPtr<AECFarmingResourceActor> FarmingTarget;

	/** 플레이어의 장비 관리 컴포넌트를 보관합니다. */
	UPROPERTY()
	TObjectPtr<UECEquipmentManagerComponent> EquipmentManager;

	/** 파밍 중 임시로 장착한 도구를 보관합니다. */
	UPROPERTY()
	TObjectPtr<UECEquipmentInstance> FarmingTool;

	/** 파밍 중 숨겼다가 복원할 기존 무기를 보관합니다. */
	UPROPERTY()
	TObjectPtr<UECWeaponInstance> EquippedWeapon;
};
