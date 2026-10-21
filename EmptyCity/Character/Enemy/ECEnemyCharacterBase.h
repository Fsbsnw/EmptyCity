// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ECCharacterBase.h"
#include "ECEnemyCharacterBase.generated.h"

class AECProjectileBase;
class UECCombatSet;
class UECMoveSpeedSet;
class UECHealthSet;
class UWidgetComponent;
class UAIPerceptionComponent;
class UECAbilitySet;
class UECItemDropComponent;
class UStateTree;
class UDeathVisualComponent;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API AECEnemyCharacterBase : public AECCharacterBase
{
	GENERATED_BODY()
	
public:
	AECEnemyCharacterBase(const FObjectInitializer& ObjectInitializer);
	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;
	virtual void OnDeathStarted(AActor* OwningActor) override;
	virtual void OnDeathFinished(AActor* OwningActor) override;

	/** 사망 애니메이션 마지막 AnimNotify가 호출하는 함수입니다. */
	void HandleDeathAnimationFinished();

// ─────────────────────────────────────────────────────────────
// Death Dissolve
// ─────────────────────────────────────────────────────────────
private:
	/** DeathVisualComponent의 연출이 끝난 뒤 기존 사망 제거 흐름을 재개합니다. */
	void HandleDeathVisualFinished();

	/** 공격 몽타주 전환 등으로 사망 애니메이션의 마지막 AnimNotify가 누락됐을 때 디졸브를 시작합니다. */
	void HandleDeathAnimationNotifyFallback();

	/** 사망 상태와 사망 비주얼이 모두 끝났을 때 액터 제거를 실행합니다. */
	void TryFinalizeDeath();

protected:
	/** 외부 컷씬이 사망 연출을 완료했을 때 사망 상태와 제거 흐름을 마무리합니다. */
	void CompleteDeathAfterExternalVisual();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Death")
	TObjectPtr<UDeathVisualComponent> DeathVisualComponent;

	/** 사망 애니메이션 종료 노티파이가 오지 않을 때 디졸브를 대신 시작할 대기 시간입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Death", meta = (ClampMin = "0.0", Units = "s"))
	float DeathAnimationNotifyFallbackDelay = 3.0f;

private:
	FTimerHandle DeathAnimationNotifyFallbackTimerHandle;
	bool bDeathStateFinished = false;
	bool bDeathVisualStarted = false;
	bool bDeathVisualFinished = false;
	bool bDeathCleanupStarted = false;

// ─────────────────────────────────────────────────────────────
// Item Drop
// ─────────────────────────────────────────────────────────────
protected:
	/** 사망 시 아이템 드롭을 생성하는 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UECItemDropComponent> ItemDropComponent;

private:
	/** 월드 메시가 없는 드롭을 직접 지급할 마지막 유효 공격자입니다. */
	TWeakObjectPtr<AActor> LastDamageInstigator;

// ─────────────────────────────────────────────────────────────
// AI
// ─────────────────────────────────────────────────────────────	

public:
	/** 이 적 클래스가 사용할 StateTree 에셋을 반환합니다. */
	UStateTree* GetStateTreeAsset() const { return StateTreeAsset; }

protected:
	/**
	 * 이 캐릭터 클래스가 사용할 StateTree 에셋입니다.
	 * 각 Enemy Character Blueprint의 Class Defaults에서 지정합니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|StateTree")
	TObjectPtr<UStateTree> StateTreeAsset = nullptr;


// ─────────────────────────────────────────────────────────────
// Attribute Method
// ─────────────────────────────────────────────────────────────
private:
	/** Default Attributes들을 적용하는 함수입니다. */
	void InitializeDefaultAttributes();

	/** Attribute Effect를 적용하는 함수입니다. */
	void ApplyAttributeEffectToSelf(const TSubclassOf<UGameplayEffect>& GameplayEffectClass, float Level);

// ─────────────────────────────────────────────────────────────
// Attribute Variable
// ─────────────────────────────────────────────────────────────
protected:
	// 체력 Attribute Set
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UECHealthSet> HealthSet;

	// 이동속도 Attribute Set
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UECMoveSpeedSet> MoveSpeedSet;
	
	// 전투 Attribute Set
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UECCombatSet> CombatSet;

	/** MoveSpeed Attribute가 변경될 때마다 CharacterMovementComponent에 적용해 주는 함수 */
	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);

	/** MoveSpeed, Attack 등의 주요 속성들을 관리합니다 */
	UPROPERTY(EditAnywhere, Category = "GAS|Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	/** 체력을 관리합니다. */
	UPROPERTY(EditAnywhere, Category = "GAS|Attributes")
	TSubclassOf<UGameplayEffect> DefaultMaxVitalAttributes;
	

// ─────────────────────────────────────────────────────────────
// HealthBar Component
// ─────────────────────────────────────────────────────────────
private:
	void InitializeHealthBarComponent();

	/** 체력이 감소하면 체력바를 표시하고 숨김 타이머를 갱신합니다. */
	UFUNCTION()
	void HandleHealthChanged(
		UECHealthComponent* ChangedHealthComponent,
		float OldValue,
		float NewValue,
		AActor* DamageInstigator);

	/** 체력바를 숨깁니다. */
	void HideHealthBar();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	/** 마지막 피격 후 체력바가 표시되는 시간입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Widget", meta = (ClampMin = "0.0", Units = "s"))
	float HealthBarVisibleDuration = 5.0f;

private:
	FTimerHandle HealthBarHideTimerHandle;

// ─────────────────────────────────────────────────────────────
// HitReaction Component
// ─────────────────────────────────────────────────────────────
private:
	void InitializeHitReactionComponent();
	
// ─────────────────────────────────────────────────────────────
// Ability Method
// ─────────────────────────────────────────────────────────────	
private:	
	/** 적이 기본으로 사용하는 어빌리티를 등록하는 함수입니다. */
	void InitAbilities();

// ─────────────────────────────────────────────────────────────
// Ability Variable
// ─────────────────────────────────────────────────────────────
private:
	/** 적이 기본으로 사용하는 어빌리티에 대한 정보입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities")
	TArray<TObjectPtr<UECAbilitySet>> DefaultAbilitySets;
};
