#include "ECHealthComponent.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "AbilitySystem/Attribute/ECHealthSet.h"
#include "ECGameplayTags.h"

UECHealthComponent::UECHealthComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
	
	ASC = nullptr;
	HealthSet = nullptr;
	DeathState = EDeathState::NotDead;
}

void UECHealthComponent::OnUnregister()
{
	UninitializeWithAbilitySystem();
	
	Super::OnUnregister();
}

void UECHealthComponent::InitializeWithAbilitySystem(UECAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);
	
	// AbilitySystemComponent 및 HealthSet을 초기화합니다.
	ASC = InASC;
	HealthSet = ASC->GetSet<UECHealthSet>();

	// HealthSet의 델리게이트에 이벤트를 등록합니다.
	HealthSet->OnHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);
	HealthSet->OnMaxHealthChanged.AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);

	// 현재 체력을 최대 체력 수치로 초기화합니다.
	ASC->SetNumericAttributeBase(UECHealthSet::GetHealthAttribute(), HealthSet->GetMaxHealth());
	
	// 사망 상태 관련 태그를 초기화합니다.
	ClearGameplayTags();
	
	// HealthSet외의 다른 클래스는 HealthComponent의 델리게이트를 중심으로 소통합니다.
	// 즉, HealthComponent에 이벤트를 등록하고, HealthComponent의 델리게이트를 브로드캐스팅 하는 책임은 HealthSet에게 전달합니다.
	OnHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
	OnMaxHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
}

void UECHealthComponent::UninitializeWithAbilitySystem()
{
	ClearGameplayTags();
	
	// 바인딩된 델리게이트를 모두 제거합니다.
	if (HealthSet)
	{
		HealthSet->OnHealthChanged.RemoveAll(this);
		HealthSet->OnMaxHealthChanged.RemoveAll(this);
		HealthSet->OnOutOfHealth.RemoveAll(this);
	}

	HealthSet = nullptr;
	ASC = nullptr;
}

float UECHealthComponent::GetHealth() const
{
	return (HealthSet ? HealthSet->GetHealth() : 0.0f);
}

float UECHealthComponent::GetMaxHealth() const
{
	return (HealthSet ? HealthSet->GetMaxHealth() : 0.0f);
}

float UECHealthComponent::GetHealthNormalized() const
{
	if (HealthSet)
	{
		const float Health = HealthSet->GetHealth();
		const float MaxHealth = HealthSet->GetMaxHealth();

		return ((MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f);
	}

	return 0.0f;
}

void UECHealthComponent::HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void UECHealthComponent::HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnMaxHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void UECHealthComponent::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	if (ASC && DamageEffectSpec)
	{
		FGameplayEventData Payload;
		Payload.EventTag = ECGameplayTags::GameplayEvent_Death;
		Payload.Instigator = DamageInstigator;
		Payload.Target = ASC->GetAvatarActor();
		Payload.OptionalObject = DamageEffectSpec->Def;
		Payload.ContextHandle = DamageEffectSpec->GetEffectContext();
		Payload.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
		Payload.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
		Payload.EventMagnitude = DamageMagnitude;

		ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
	}
}

void UECHealthComponent::StartDeath()
{
	if (DeathState != EDeathState::NotDead)
	{
		return;
	}

	DeathState = EDeathState::DeathStarted;

	if (ASC)
	{
		ASC->SetLooseGameplayTagCount(ECGameplayTags::Status_Death_Dying, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathStarted.Broadcast(Owner);
}

void UECHealthComponent::FinishDeath()
{
	if (DeathState != EDeathState::DeathStarted)
	{
		return;
	}

	DeathState = EDeathState::DeathFinished;

	if (ASC)
	{
		ASC->SetLooseGameplayTagCount(ECGameplayTags::Status_Death_Dead, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathFinished.Broadcast(Owner);
}

void UECHealthComponent::ClearGameplayTags()
{
	if (ASC)
	{
		ASC->SetLooseGameplayTagCount(ECGameplayTags::Status_Death_Dying, 0);
		ASC->SetLooseGameplayTagCount(ECGameplayTags::Status_Death_Dead, 0);
	}
}
