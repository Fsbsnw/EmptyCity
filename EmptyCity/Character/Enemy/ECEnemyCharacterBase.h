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

// ─────────────────────────────────────────────────────────────
// Item Drop
// ─────────────────────────────────────────────────────────────
protected:
	/** 사망 시 아이템 드롭을 생성하는 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UECItemDropComponent> ItemDropComponent;

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

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TObjectPtr<UWidgetComponent> HealthBarComponent;

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
