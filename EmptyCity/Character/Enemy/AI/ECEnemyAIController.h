// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ECEnemyAIController.generated.h"

class UAISenseConfig_Sight;
struct FAIStimulus;
class UBehaviorTreeComponent;
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
	virtual void BeginPlay() override;

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
	float SleepDistanceThreshold = 3000.0f; // 이 거리보다 멀어지면 BT를 일시정지합니다.

private:
	/** 매번 찾지 않도록 타겟 플레이어 폰을 캐싱 */
	UPROPERTY(Transient)
	TWeakObjectPtr<APawn> CachedPlayerPawn;

// ─────────────────────────────────────────────────────────────
// AI Component
// ─────────────────────────────────────────────────────────────
protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	/** 시야 센서 설정 변수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

private:
	/** 감지 이벤트가 발생할 때 실행될 함수 */
	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);
};
