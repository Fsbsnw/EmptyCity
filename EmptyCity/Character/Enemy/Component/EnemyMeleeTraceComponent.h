#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyMeleeTraceComponent.generated.h"

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

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UEnemyMeleeTraceComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UEnemyMeleeTraceComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime,	ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 지정한 공격 부위의 근접 공격을 활성화합니다. */
	void BeginTrace(EEnemyMeleeTracePart TracePart);

	/** 근접 공격을 종료합니다. */
	void EndTrace();

	/** 공격 부위별 소켓 및 Sphere Sweep 설정입니다. */
	UPROPERTY(EditDefaultsOnly, Category="Trace")
	TMap<EEnemyMeleeTracePart, FEnemyMeleeTraceConfig> TracePartConfigs;
	
private:
	/** 근접 공격을 틱마다 추적합니다. */
	void PerformTrace();

// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
private:
	/** 현재 공격에서 사용하는 트레이스 설정입니다. */
	FEnemyMeleeTraceConfig ActiveTraceConfig;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> OwnerMesh;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> HitActors;

	FVector PreviousTraceStart = FVector::ZeroVector;
	FVector PreviousTraceEnd = FVector::ZeroVector;

	/** 트레이스 활성화를 관리합니다. */
	bool bAttackTracing = false;
	
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
