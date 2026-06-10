#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ECHealthComponent.generated.h"

class UECHealthSet;
class UECAbilitySystemComponent;
struct FGameplayEffectSpec;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealth_DeathEvent, AActor*, OwningActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FHealth_AttributeChanged, UECHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);

UENUM(BlueprintType)
enum class EDeathState : uint8
{
	NotDead = 0,
	DeathStarted,
	DeathFinished
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EMPTYCITY_API UECHealthComponent : public UActorComponent
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// ActorComponent Interface
// ─────────────────────────────────────────────────────────────		
public:
	UECHealthComponent();
	virtual void OnUnregister() override;
	
	
// ─────────────────────────────────────────────────────────────
// Delegate
// ─────────────────────────────────────────────────────────────			
public:
	/** 현재 체력의 변화 감지용 델리게이트입니다. */
	FHealth_AttributeChanged OnHealthChanged;
	
	/** 최대 체력의 변화 감지용 델리게이트입니다. */
	FHealth_AttributeChanged OnMaxHealthChanged;
	
	/** 사망 상태 시작 감지용 델리게이트입니다. */
	FHealth_DeathEvent OnDeathStarted;
	
	/** 사망 상태 종료 감지용 델리게이트입니다. */
	FHealth_DeathEvent OnDeathFinished;

	
// ─────────────────────────────────────────────────────────────
// AbilitySystem Initialize
// ─────────────────────────────────────────────────────────────		
public:
	/** Owner의 AbilitySystem으로 데이터를 초기화하기 위한 함수입니다. */
	void InitializeWithAbilitySystem(UECAbilitySystemComponent* InASC);
	
	/** 사망, 월드 초기화 등의 이유로 데이터를 제거하기 위한 함수입니다. */
	void UninitializeWithAbilitySystem();

	
// ─────────────────────────────────────────────────────────────
// Delegate Binding Method
// ─────────────────────────────────────────────────────────────	
protected:
	/** 현재 체력의 변화 감지용 델리게이트 바인딩 함수입니다. */
	virtual void HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	
	/** 최대 체력의 변화 감지용 델리게이트 바인딩 함수입니다. */
	virtual void HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	
	/** 사망 상태 감지용 델리게이트 바인딩 함수입니다. */
	virtual void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);


// ─────────────────────────────────────────────────────────────
// Death State Method
// ─────────────────────────────────────────────────────────────	
public:
	/** 사망 상태 시작 시 호출되는 함수입니다. */
	virtual void StartDeath();

	/** 사망 상태 종료 시 호출되는 함수입니다. */
	virtual void FinishDeath();
	
	/** 사망 상태 태그를 초기화하기 위한 함수입니다. Initialize/Uninitialize시 호출됩니다. */
	void ClearGameplayTags();

	
// ─────────────────────────────────────────────────────────────
// Utilization
// ─────────────────────────────────────────────────────────────		
public:
	/** 캐릭터가 소유한 HealthComponent를 반환하는 헬퍼 함수입니다. */
	static UECHealthComponent* FindHealthComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UECHealthComponent>() : nullptr); } 
	
	/** 현재 체력을 반환합니다. */
	float GetHealth() const;
	
	/** 최대 체력을 반환합니다. */
	float GetMaxHealth() const;
	
	/** 최대 체력을 기준으로 현재 체력의 비율을 반환합니다. */
	float GetHealthNormalized() const;

	/** 현재 사망 상태를 반환합니다. Death Ability에서 활용합니다. */
	EDeathState GetDeathState() const { return DeathState; }
	
	
// ─────────────────────────────────────────────────────────────
// Cache Variable
// ─────────────────────────────────────────────────────────────
protected:
	/** Owner의 AbilitySystemComponent를 캐싱해놓은 변수입니다. */
	UPROPERTY()
	TObjectPtr<UECAbilitySystemComponent> ASC;

	/** Owner의 HealthSet을 캐싱해놓은 변수입니다. */
	UPROPERTY()
	TObjectPtr<const UECHealthSet> HealthSet;
	
	/** HealthComponent 내부에서 캐릭터의 사망 상태를 추적하기 위한 Enum 변수입니다. */
	EDeathState DeathState;
	
};