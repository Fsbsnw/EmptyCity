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

// ─────────────────────────────────────────────────────────────
// Hit Stop
// ─────────────────────────────────────────────────────────────
public:
	/** 데미지 처리 구간에서 히트 스탑을 처리합니다. */
	void PlayHitStop(float TimeDuration = 0.3f, float TimeDilation = 0.05f);

	UPROPERTY(EditAnywhere) // 블루프린트 테스트용
	bool bCanPlayHitStop = true;

private:
	/** 히트 스탑이 완료된 후에 딜레이를 원상복구합니다. */
	void RestoreTimeDilation();

	/** 다단 히트 시 타이머를 덮어씌우기 위한 타이머 핸들 */
	FTimerHandle HitStopTimerHandle;
};
