#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "WeaponTraceComponent.generated.h"

class UECWeaponInstance;
class UAnimMontage;
class UGameplayEffect;
class UMeshComponent;

/** 무기 Sweep 판정에 사용하는 설정입니다. */
USTRUCT(BlueprintType)
struct FECWeaponTraceConfig
{
	GENERATED_BODY()

	/** 판정 시작 소켓입니다. */
	UPROPERTY(EditDefaultsOnly)
	FName TraceStartSocket = TEXT("TraceStart");

	/** 판정 끝 소켓입니다. */
	UPROPERTY(EditDefaultsOnly)
	FName TraceEndSocket = TEXT("TraceEnd");

	/** Sweep 구체의 반지름입니다. */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.1"))
	float TraceRadius = 8.0f;

	/** 무기 길이를 따라 검사할 지점 수입니다. */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "2"))
	int32 SampleCount = 4;
};

/** 무기에서 지원하는 공격 한 종류의 데이터입니다. */
USTRUCT(BlueprintType)
struct FECWeaponAttackDefinition
{
	GENERATED_BODY()

	/** 이 공격을 식별하는 태그입니다. PlayerAttack Ability의 공격 태그와 일치해야 합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "공격", meta = (Categories = "Ability.Type.Action.Attack"))
	FGameplayTag AttackTag;

	/** 공격 시 재생할 몽타주입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "공격")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** 타격 대상에게 적용할 GameplayEffect입니다. 실제 적용은 공격 판정 단계에서 처리합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "공격")
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	/** 이 공격에 사용할 Sweep 설정입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "공격")
	FECWeaponTraceConfig TraceConfig;
};

/** 공격 구간 동안 무기 소켓을 기준으로 판정을 수행하는 컴포넌트입니다. */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EMPTYCITY_API UWeaponTraceComponent : public UActorComponent
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// ActorComponent Interface
// ─────────────────────────────────────────────────────────────
public:
	UWeaponTraceComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



// ─────────────────────────────────────────────────────────────
// Attack Method
// ─────────────────────────────────────────────────────────────
public:
	/** AttackTag에 대응하는 공격을 현재 공격으로 선택하고 정의를 반환합니다. */
	const FECWeaponAttackDefinition* SetActiveAttack(const FGameplayTag& AttackTag);

	/** AttackTag가 정확히 일치하는 공격 정의를 반환합니다. */
	const FECWeaponAttackDefinition* FindAttackDefinition(const FGameplayTag& AttackTag) const;

	/** 현재 공격과 진행 중인 판정 상태를 정리합니다. */
	void ClearActiveAttack();

	/** 공격 판정을 시작하고 컴포넌트 Tick을 활성화합니다. */
	void BeginTrace();

	/** 공격 판정을 종료하고 컴포넌트 Tick을 비활성화합니다. */
	void EndTrace();

private:
	/** 활성화된 공격 정의를 사용해 이번 프레임의 판정을 처리합니다. */
	void UpdateTrace();

	/** 현재 선택된 공격 정의를 반환합니다. */
	const FECWeaponAttackDefinition* GetActiveAttackDefinition() const;

	/** 두 지점 사이를 구체로 Sweep하고 새 적중 대상을 기록합니다. */
	void PerformSweep(const FVector& Start, const FVector& End, float Radius);

	/** 한 번의 Sweep 범위와 적중 지점을 표시합니다. */
	void DrawDebugSweep(const FVector& Start, const FVector& End, float Radius, const TArray<FHitResult>& HitResults) const;
	
	
	
// ─────────────────────────────────────────────────────────────
// Attack Method
// ─────────────────────────────────────────────────────────────
public:
	void SetOwningWeaponInstance(UECWeaponInstance* InWeaponInstance);

	/** 적중 대상에게 현재 무기의 데미지 GameplayEffect를 적용합니다. */
	void ApplyDamageToHitTarget(const FHitResult& HitResult);

	
// ─────────────────────────────────────────────────────────────
// Attack Variable
// ─────────────────────────────────────────────────────────────
protected:
	/** 이 무기가 지원하는 공격 목록입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|공격", meta = (TitleProperty = "AttackTag"))
	TArray<FECWeaponAttackDefinition> AttackDefinitions;

	/** 현재 공격에서 이미 감지한 액터 목록입니다. */
	TSet<TWeakObjectPtr<AActor>> HitActors;

	/** 공격 판정에 사용할 무기 메시입니다. */
	TWeakObjectPtr<UMeshComponent> TraceMesh;

	/** 현재 선택된 공격 태그입니다. */
	FGameplayTag ActiveAttackTag;

	/** 이전 프레임의 시작 소켓 위치입니다. */
	FVector PreviousTraceStart = FVector::ZeroVector;

	/** 이전 프레임의 끝 소켓 위치입니다. */
	FVector PreviousTraceEnd = FVector::ZeroVector;

	/** 공격 판정 활성 여부입니다. */
	bool bTraceActive = false;

	/** 이전 소켓 위치 저장 여부입니다. */
	bool bHasPreviousTracePositions = false;

	/** Sweep 범위와 적중 지점을 화면에 표시할지 결정합니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|디버그")
	bool bDrawDebugTrace = false;

	
	
// ─────────────────────────────────────────────────────────────
// Cached Variable
// ─────────────────────────────────────────────────────────────
private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UECWeaponInstance> OwningWeaponInstance;
	
};
