// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "StaminaViewModel.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;
/**
 * @brief 캐릭터의 스태미나(Stamina) 속성을 UI(View)에 바인딩하기 위한 MVVM 뷰모델 클래스
 */
UCLASS()
class EMPTYCITY_API UStaminaViewModel : public UECViewModelBase
{
	GENERATED_BODY()
public:
	virtual void BindCallbacksToDependencies(AActor* ContextActor) override;
	virtual void BroadcastInitialValues() override;
	virtual void BeginDestroy() override;

public: 
// ============================================================================
// MVVM 필드 노티파이용 Getters & Setters
// ============================================================================
	
	int32 GetCurrentStamina() const { return CurrentStamina; }
	void SetCurrentStamina(int32 NewStamina);

	int32 GetMaxStamina() const { return MaxStamina; }
	void SetMaxStamina(int32 NewMaxStamina);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Stamina")
	float GetStaminaPercent() const;
	
private:
// ============================================================================
// View 바인딩 속성
// ============================================================================

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess))
	int32 CurrentStamina;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess))
	int32 MaxStamina;

// ============================================================================
// View 바인딩 델리게이트
// ============================================================================
	
	void HandleStaminaChanged(const FOnAttributeChangeData& Data);
	void HandleMaxStaminaChanged(const FOnAttributeChangeData& Data);
	void UnbindAttributeDelegates();

	UPROPERTY(Transient)
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	FDelegateHandle StaminaChangedHandle;
	FDelegateHandle MaxStaminaChangedHandle;
};
