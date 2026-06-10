// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ECCharacterBase.h"
#include "ECEnemyCharacterBase.generated.h"

class UECMoveSpeedSet;
class UECHealthSet;
class UWidgetComponent;
class UAIPerceptionComponent;
class UECAbilitySet;
class UBehaviorTree;
class UBehaviorTreeComponent;
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

// ─────────────────────────────────────────────────────────────
// AI
// ─────────────────────────────────────────────────────────────	

public:
	UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;


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
	// 반드시 UPROPERTY를 붙여서 주인이 있다는 것을 엔진에 알려야 합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UECHealthSet> HealthSet;

	// 반드시 UPROPERTY를 붙여서 주인이 있다는 것을 엔진에 알려야 합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UECMoveSpeedSet> MoveSpeedSet;

	/** MoveSpeed Attribute가 변경될 때마다 CharacterMovementComponent에 적용해 주는 함수 */
	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);

	/** MoveSpeed, Attack 등의 주요 속성들을 관리합니다 */
	UPROPERTY(EditAnywhere, Category = "GAS|Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	/** 체력을 관리합니다. */
	UPROPERTY(EditAnywhere, Category = "GAS|Attributes")
	TSubclassOf<UGameplayEffect> DefaultMaxVitalAttributes;
	

// ─────────────────────────────────────────────────────────────
// HealthBar Method
// ─────────────────────────────────────────────────────────────
private:
	void InitializeHealthBarComponent();

// ─────────────────────────────────────────────────────────────
// HealthBar Widget Component
// ─────────────────────────────────────────────────────────────
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TObjectPtr<UWidgetComponent> HealthBarComponent;
	
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
	UPROPERTY(EditDefaultsOnly, Category = "변수|어빌리티")
	TArray<TObjectPtr<UECAbilitySet>> DefaultAbilitySets;
};
