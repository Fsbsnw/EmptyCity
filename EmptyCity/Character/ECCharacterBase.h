#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "ECCharacterBase.generated.h"

class UECHealthComponent;

UCLASS()
class EMPTYCITY_API AECCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// Character Interface
// ─────────────────────────────────────────────────────────────	
public:
	AECCharacterBase(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaSeconds) override;
	
	
// ─────────────────────────────────────────────────────────────
// AbilitySystem Interface
// ─────────────────────────────────────────────────────────────		
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return ASC; }
	
	
// ─────────────────────────────────────────────────────────────
// Tag Update
// ─────────────────────────────────────────────────────────────
protected:
	void UpdateMovingTag();
	
	
// ─────────────────────────────────────────────────────────────
// Death State Method
// ─────────────────────────────────────────────────────────────	
protected:
	/** HealthComponent의 OnDeathStarted에 바인딩할 함수입니다. */
	UFUNCTION()
	virtual void OnDeathStarted(AActor* OwningActor);
	
	/** HealthComponent의 OnDeathFinished에 바인딩할 함수입니다. */
	UFUNCTION()
	virtual void OnDeathFinished(AActor* OwningActor);
	
	/** 플레이어의 이동과 콜리전을 비활성화하는 함수입니다. */
	void DisableMovementAndCollision();
	
	/** 사망 연출을 출력하는 함수입니다. */
	void StartDeathProcess();
	
	/** OnDeathFinished에서 한 틱 후 호출하는 함수입니다. 실질적인 Destory는 여기서 담당합니다. */
	void DestroyDueToDeath();
	
	
// ─────────────────────────────────────────────────────────────
// Component
// ─────────────────────────────────────────────────────────────	
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UECHealthComponent> HealthComponent;
	
	
// ─────────────────────────────────────────────────────────────
// Cache Variable
// ─────────────────────────────────────────────────────────────		
protected:
	UPROPERTY()
	TObjectPtr<UECAbilitySystemComponent> ASC;
	
	/** 직전 프레임의 이동 여부 — 상태가 바뀐 프레임에만 태그를 갱신하기 위한 캐시입니다. */
	bool bIsMoving = false;
	
};
