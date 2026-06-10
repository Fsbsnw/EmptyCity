// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/ECEnemyCharacterBase.h"
#include "ECEnemyElite.generated.h"

class AECEnemyNormal;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API AECEnemyElite : public AECEnemyCharacterBase
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;	
	virtual void OnDeathFinished(AActor* OwningActor) override;

// ─────────────────────────────────────────────────────────────
// Enemy Normal Management
// ─────────────────────────────────────────────────────────────	
public:
	/** Death 테스트 용도의 함수 */
	UFUNCTION(BlueprintCallable)
	void DeathTest() { OnDeathFinished(this); }
	
	/** 캐싱된 주변 일반 몬스터 배열을 반환합니다. */
	const TArray<AECEnemyNormal*>& GetManagedMinions() const { return ManagedMinions; }

protected:
	/** BeginPlay 시점에 주변의 일반 몬스터를 탐색하여 배열에 캐싱합니다. */
	void CachingManagedMinions();

	/** 캐싱된 일반 몬스터들에게 엘리트 전용 버프(GA)를 부여하도록 트리거합니다. */
	void GrantEliteBuffToMinions();

protected:
	/** 주변 일반 몬스터를 탐색할 반경 */
	UPROPERTY(EditDefaultsOnly, Category = "Elite|Minion")
	float ManageRadius = 1500.f;

private:
	/** 현재 엘리트 몬스터가 관리 중인 일반 몬스터 목록 */
	UPROPERTY(VisibleAnywhere, Category = "Elite|Minion")
	TArray<AECEnemyNormal*> ManagedMinions;
};
