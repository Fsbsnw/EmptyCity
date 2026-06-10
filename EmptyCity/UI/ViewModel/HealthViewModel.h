// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "HealthViewModel.generated.h"

struct FGameplayEffectSpec;
/**
 * @brief 캐릭터의 체력(Health) 속성을 UI(View)에 바인딩하기 위한 MVVM 뷰모델 클래스
 */
UCLASS()
class EMPTYCITY_API UHealthViewModel : public UECViewModelBase
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies(AActor* ContextActor) override;
	virtual void BroadcastInitialValues() override;

	/**
	 * @brief [예시/미정] 뷰모델이 참조할 GAS 의존성(ASC, AttributeSet)을 주입합니다.
	 * @note 이 함수는 컨트롤러나 액터가 뷰모델을 생성한 직후 가장 먼저 호출해야 합니다.
	 * 
	 * [구현 예시]
	 * void UHealthViewModel::SetAbilitySystemDependencies(UAbilitySystemComponent* InASC, UAttributeSet* InAS)
	 * {
	 *     AbilitySystemComponent = InASC;
	 *     AttributeSet = InAS;
	 *     
	 *     // 의존성이 주입되면 즉시 콜백을 연결하고 초기 값을 전달합니다.
	 *     BindCallbacksToDependencies();
	 *     BroadcastInitialValues();
	 * }
	 * => BindCallbacksToDependencies(AActor* ContextActor) 이걸로 교체했으니,
	 * ContextActor를 캐스팅해서 필요한 데이터를 바인딩 해야됩니다.
	 *
	 * // BindCallbacksToDependencies()에서 아래 함수들로 바인딩 합니다.
	 * void UHealthViewModel::OnHealthChanged(const FOnAttributeChangeData& Data)
	 * {
	 *     SetCurrentHealth(Data.NewValue);
	 * }
	 * 
	 * void UHealthViewModel::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
	 * {
	 *     SetMaxHealth(Data.NewValue);
	 * }
	 */

public: 
// ============================================================================
// MVVM 필드 노티파이용 Getters & Setters
// ============================================================================
	
	int32 GetCurrentHealth() const { return CurrentHealth; }
	void SetCurrentHealth(int32 NewHealth);

	int32 GetMaxHealth() const { return MaxHealth; }
	void SetMaxHealth(int32 NewMaxHealth);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Health")
	float GetHealthPercent() const;
	
private:
// ============================================================================
// View 바인딩 속성
// ============================================================================

	/** 아래의 프로퍼티 지정자를 엄격하게 준수해야 됩니다. */
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess))
	int32 CurrentHealth;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess))
	int32 MaxHealth;

// ============================================================================
// View 바인딩 델리게이트
// ============================================================================
	// 델리게이트 시그니처와 동일한 6개의 매개변수를 갖는 함수 선언
	void HandleHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue);
	void HandleMaxHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue);
};
