// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Components/StateTreeAIComponent.h"
#include "ECEnemyAIController.generated.h"

class UAISenseConfig_Sight;
class UStateTree;
struct FAIStimulus;

/**
 * 적 캐릭터가 보유한 StateTree 에셋을 런타임에 적용하기 위한 컴포넌트입니다.
 */
UCLASS(ClassGroup = AI)
class EMPTYCITY_API UECEnemyStateTreeAIComponent : public UStateTreeAIComponent
{
	GENERATED_BODY()

public:
	void SetStateTreeAsset(UStateTree* NewStateTree);
};

/**
 * 
 */
UCLASS()
class EMPTYCITY_API AECEnemyAIController : public AAIController
{
	GENERATED_BODY()
public:
	AECEnemyAIController();
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void BeginPlay() override;
	void HandlePawnDeath();

// ─────────────────────────────────────────────────────────────
// AI LOD
// ─────────────────────────────────────────────────────────────
protected:
	// 거리를 체크할 타이머 핸들
	FTimerHandle LODTimerHandle;

	// 주기적으로 실행될 최적화 함수
	void UpdateAILOD();

	// 최적화 기준 거리 설정
	UPROPERTY(EditAnywhere, Category = "AI|Optimization")
	float SleepDistanceThreshold = 3000.0f; // 이 거리보다 멀어지면 StateTree를 일시정지합니다.

private:
	/** 매번 찾지 않도록 타겟 플레이어 폰을 캐싱 */
	UPROPERTY(Transient)
	TWeakObjectPtr<APawn> CachedPlayerPawn;

	bool bIsPawnDead = false;

// ─────────────────────────────────────────────────────────────
// AI Component
// ─────────────────────────────────────────────────────────────
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UECEnemyStateTreeAIComponent> StateTreeComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	/** 시야 센서 설정 변수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

private:
	/** 감지 이벤트가 발생할 때 실행될 함수 */
	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

public:
	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<AActor> TargetActor = nullptr;

	/** 현재 Sight로 플레이어를 직접 보고 있는지 */
	UPROPERTY(EditAnywhere, Category = "AI")
	bool bHasLineOfSight = false;

	/** 현재 전투가 유지되고 있는지 */
	UPROPERTY(EditAnywhere, Category = "AI")
	bool bIsInCombat = false;

	/** 마지막으로 플레이어를 확인한 위치 */
	UPROPERTY(EditAnywhere, Category = "AI")
	FVector LastKnownTargetLocation = FAISystem::InvalidLocation;

	/** 전투 상태에서 벗어날 때 전투 정보들을 초기화하는 함수입니다. */
	void ClearCombatTarget();
};
