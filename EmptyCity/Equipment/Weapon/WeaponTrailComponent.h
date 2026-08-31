#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "WeaponTrailComponent.generated.h"

class UMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;

/** AnimNotifyState가 지정한 구간 동안 무기의 두 소켓을 Niagara Ribbon 파라미터로 전달합니다. */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EMPTYCITY_API UWeaponTrailComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponTrailComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 트레일을 시작합니다. 중첩된 요청은 마지막 요청이 끝날 때까지 유지됩니다. */
	UFUNCTION(BlueprintCallable, Category="Weapon Trail")
	bool BeginTrail();

	/** 트레일 시작 요청 하나를 종료합니다. */
	UFUNCTION(BlueprintCallable, Category="Weapon Trail")
	void EndTrail();

	/** 몽타주 중단, 사망 등의 상황에서 모든 요청을 무시하고 트레일을 즉시 종료합니다. */
	UFUNCTION(BlueprintCallable, Category="Weapon Trail")
	void ForceEndTrail();

	UFUNCTION(BlueprintPure, Category="Weapon Trail")
	bool IsTrailActive() const { return ActiveTrailRequestCount > 0; }

protected:
	/** 소켓을 조회할 무기 메시입니다. 비워두면 두 소켓을 모두 가진 MeshComponent를 자동 탐색합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Trail|Source")
	FComponentReference SourceMeshComponent;

	/** 실행할 Niagara 트레일 시스템입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon Trail|Effect")
	TObjectPtr<UNiagaraSystem> TrailSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon Trail|Source")
	FName StartSocket = TEXT("SM_BossWeapon_A01_Bottom");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon Trail|Source")
	FName EndSocket = TEXT("SM_BossWeapon_A01_Mid");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon Trail|Parameters")
	FName TrailStartParameter = TEXT("User.TrailStart");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon Trail|Parameters")
	FName TrailEndParameter = TEXT("User.TrailEnd");

private:
	bool ResolveSourceMesh();
	bool EnsureTrailEffectComponent();
	bool UpdateTrailParameters();
	void StopTrailEffect();

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> TrailEffectComponent;

	TWeakObjectPtr<UMeshComponent> CachedSourceMesh;

	/** 인접하거나 겹친 NotifyState가 서로의 트레일을 조기에 끄지 않도록 요청 수를 추적합니다. */
	int32 ActiveTrailRequestCount = 0;
};