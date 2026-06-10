#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ActiveGameplayEffectHandle.h"
#include "ECGameplayAbility.generated.h"

class UECAbilityCost;
class AECCharacterBase;
class UECAbilitySystemComponent;
class AECPlayerController;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EAbilityActivationPolicy : uint8
{
	// Input이 Trigger 됐을 경우 (Pressed/Released)
	OnInputTriggered,
	
	// Input이 Held되어 있을 경우
	WhileInputActive,
	
	// Avatar가 생성됐을 경우 바로 할당 (패시브 스킬)
	OnSpawn
};

UENUM(BlueprintType)
enum class EAbilityActivationGroup : uint8
{
	// 다른 어빌리티의 발동 여부와 관계없이 독립적으로 발동하는 어빌리티입니다.
	Independent,

	// 다른 Exclusive_Replaceable 어빌리티를 취소하고 발동되는 어빌리티입니다.
	Exclusive_Replaceable,

	// 모든 Exclusive 어빌리티를 취소하고 발동되는 어빌리티입니다.
	Exclusive_Blocking,

	MAX	UMETA(Hidden)
};


UCLASS()
class EMPTYCITY_API UECGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// GameplayAbility Interface
// ─────────────────────────────────────────────────────────────
public:
	UECGameplayAbility();
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void SetCanBeCanceled(bool bCanBeCanceled) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void InputStarted() { };

// ─────────────────────────────────────────────────────────────
// Activation Policy
// ─────────────────────────────────────────────────────────────
protected:
	/**
	 * ActivationPolicy가 OnSpawn일 경우, 어빌리티가 부여되는 즉시 스스로 발동을 시도합니다.
	 * 패시브 스킬처럼 별도 입력 없이 켜져 있어야 하는 어빌리티를 위한 처리입니다.
	 */
	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

	
// ─────────────────────────────────────────────────────────────
// Utilization
// ─────────────────────────────────────────────────────────────	
public:
	AECPlayerController* GetECPlayerControllerFromActorInfo() const;
	UECAbilitySystemComponent* GetECAbilitySystemComponentFromActorInfo() const;
	AECCharacterBase* GetECCharacterFromActorInfo() const;
	
	
// ─────────────────────────────────────────────────────────────
// ActivationPolicy Variable
// ─────────────────────────────────────────────────────────────	
public:
	/** 어빌리티가 발동되기 위한 정책입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "변수|Activation")
	EAbilityActivationPolicy ActivationPolicy;
	
	/** 다른 어빌리티와의 관계를 나타내는 변수입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "변수|Activation")
	EAbilityActivationGroup ActivationGroup;
	

// ─────────────────────────────────────────────────────────────
// Cost
// ─────────────────────────────────────────────────────────────
protected:
	/** 어빌리티 발동 시 필요한 Instant 코스트를 나타내는 변수입니다. */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "변수|Cost")
	TArray<TObjectPtr<UECAbilityCost>> AdditionalCosts;
	
	/**
	 * 어빌리티가 활성화된 동안 자동으로 Apply되고, 종료 시 자동으로 Remove되는 Periodic GE 목록입니다.
	 * 매그니튜드는 GE의 Modifier(AttributeBasedFloat 등)에서 결정되며, 외부 주입 없이 적용됩니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "변수|Periodic")
	TArray<TSubclassOf<UGameplayEffect>> PeriodicEffects;

private:
	/** 현재 활성화 중 적용된 Periodic GE 핸들 목록입니다. EndAbility 시 일괄 제거됩니다. */
	TArray<FActiveGameplayEffectHandle> AppliedPeriodicEffectHandles;
	
	
// ─────────────────────────────────────────────────────────────
// Cache
// ─────────────────────────────────────────────────────────────
private:
	/** 입력이 들어온 순간을 추적하기 위한 변수입니다. */
	uint8 bIsPressed : 1 = false;

};