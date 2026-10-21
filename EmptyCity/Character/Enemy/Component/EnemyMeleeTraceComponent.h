#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyMeleeTraceComponent.generated.h"

class UMaterialInterface;
struct FHitResult;

UENUM(BlueprintType)
enum class EEnemyMeleeTracePart : uint8
{
	Weapon		UMETA(DisplayName = "장착 무기"),
	RightFoot	UMETA(DisplayName = "오른쪽 발"),
};

USTRUCT(BlueprintType)
struct FEnemyMeleeTraceConfig
{
	GENERATED_BODY()

	/** 공격 판정 구간의 시작 소켓입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName StartSocket = NAME_None;

	/** 공격 판정 구간의 끝 소켓입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName EndSocket = NAME_None;

	/** 각 소켓의 이동 경로를 검사할 Sphere Sweep 반경입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float TraceRadius = 20.0f;
};

/** 한 공격 애니메이션의 무기 지면 충돌 데칼 설정입니다. */
USTRUCT(BlueprintType)
struct FEnemyWeaponGroundDecalConfig
{
	GENERATED_BODY()

	/** 무기 트레이스가 지면에 닿았을 때 데칼을 생성합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEnabled = false;

	/** 생성할 Deferred Decal 재질입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bEnabled", EditConditionHides))
	TObjectPtr<UMaterialInterface> DecalMaterial;

	/** 데칼 투영 깊이(X), 너비(Y), 높이(Z)입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bEnabled", EditConditionHides, ClampMin="0.0"))
	FVector DecalSize = FVector(16.0f, 200.0f, 200.0f);

	/** 무기 진행 방향을 기준으로 더할 회전입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bEnabled", EditConditionHides))
	FRotator RotationOffset = FRotator::ZeroRotator;

	/** 이 값보다 ImpactNormal.Z가 작은 벽면에는 데칼을 생성하지 않습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bEnabled", EditConditionHides, ClampMin="-1.0", ClampMax="1.0"))
	float MinimumGroundNormalZ = 0.5f;

	/** 0이면 자동 제거하지 않습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bEnabled", EditConditionHides, ClampMin="0.0", Units="s"))
	float LifeSpan = 3.0f;

	/** 수명 종료 전에 페이드 아웃하는 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bEnabled", EditConditionHides, ClampMin="0.0", Units="s"))
	float FadeOutDuration = 1.0f;

	/** 움직이는 바닥 컴포넌트에도 데칼이 따라가도록 부착합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bEnabled", EditConditionHides))
	bool bAttachToHitComponent = true;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UEnemyMeleeTraceComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UEnemyMeleeTraceComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime,	ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 지정한 공격 부위의 근접 공격과 이 애니메이션의 무기 데칼 설정을 활성화합니다. */
	void BeginTrace(
		EEnemyMeleeTracePart TracePart,
		const FEnemyWeaponGroundDecalConfig& WeaponGroundDecalConfig);

	/** 근접 공격을 종료합니다. */
	void EndTrace();

	/** 공격 부위별 소켓 및 Sphere Sweep 설정입니다. */
	UPROPERTY(EditDefaultsOnly, Category="Trace")
	TMap<EEnemyMeleeTracePart, FEnemyMeleeTraceConfig> TracePartConfigs;
	
private:
	/** 근접 공격을 틱마다 추적합니다. */
	void PerformTrace();

	/** 무기 Sweep이 바닥에 닿았을 때 현재 애니메이션 설정으로 데칼을 생성합니다. */
	void TrySpawnWeaponGroundDecal(
		const FHitResult& HitResult,
		const FVector& SwingDirection,
		const FVector& WeaponDirection);

// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
private:
	/** 현재 공격에서 사용하는 트레이스 설정입니다. */
	FEnemyMeleeTraceConfig ActiveTraceConfig;

	/** 현재 공격 애니메이션에서 사용하는 무기 지면 데칼 설정입니다. */
	FEnemyWeaponGroundDecalConfig ActiveWeaponGroundDecalConfig;

	/** 현재 활성화된 공격 부위입니다. */
	EEnemyMeleeTracePart ActiveTracePart = EEnemyMeleeTracePart::Weapon;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> OwnerMesh;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> HitActors;

	FVector PreviousTraceStart = FVector::ZeroVector;
	FVector PreviousTraceEnd = FVector::ZeroVector;

	/** 트레이스 활성화를 관리합니다. */
	bool bAttackTracing = false;

	/** 한 번의 공격 트레이스에서 바닥 데칼이 중복 생성되는 것을 막습니다. */
	bool bWeaponGroundDecalSpawned = false;
	
// ─────────────────────────────────────────────────────────────
// Debug
// ─────────────────────────────────────────────────────────────
private:
	void DrawTraceDebug(const FVector& Start, const FVector& End, float Radius, bool bHit) const;
	
	UPROPERTY(EditDefaultsOnly, Category="Debug", meta=(AllowPrivateAccess="true"))
	bool bDrawDebug = false;

	UPROPERTY(EditDefaultsOnly, Category="Debug", meta=(ClampMin="0.0", AllowPrivateAccess="true"))
	float DebugLifetime = 0.05f;
};
