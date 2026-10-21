#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "BossPatternComponent.generated.h"

class AECBossGroundHazard;
class UAbilitySystemComponent;
class UECHealthComponent;
class UGameplayEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FECBossPatternEvent,
	class UECBossPatternComponent*,
	PatternComponent);

/**
 * 보스 체력에 따라 지면 장판을 한 번 준비하고 순차 폭발시키는 패턴 컴포넌트입니다.
 * 기본 흐름은 75%에서 장판 준비, 50%에서 폭발 요청이며 모든 런타임 상태는 보스 인스턴스에 귀속됩니다.
 */
UCLASS(ClassGroup=(Boss), meta=(BlueprintSpawnableComponent))
class EMPTYCITY_API UECBossPatternComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UECBossPatternComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 50% 스크림의 정확한 프레임에 AnimNotify 또는 Ability가 호출할 수 있습니다. */
	UFUNCTION(BlueprintCallable, Category="Boss Pattern")
	void StartSequentialDetonation();

	/** 재도전 등 새 전투를 위해 장판과 1회성 상태를 초기화합니다. */
	UFUNCTION(BlueprintCallable, Category="Boss Pattern")
	void ResetPattern();

	/** AI 타겟 대신 장판 중심으로 사용할 액터를 명시적으로 지정합니다. */
	UFUNCTION(BlueprintCallable, Category="Boss Pattern")
	void SetPatternTarget(AActor* NewTarget);

	UFUNCTION(BlueprintPure, Category="Boss Pattern")
	bool AreHazardsPrepared() const { return bHazardsPrepared; }

	UFUNCTION(BlueprintPure, Category="Boss Pattern")
	bool IsDetonationRequested() const { return bDetonationRequested; }

	UFUNCTION(BlueprintPure, Category="Boss Pattern")
	bool HasDetonationStarted() const { return bDetonationStarted; }

	/** 75% 준비가 완료됐을 때 발생합니다. */
	UPROPERTY(BlueprintAssignable, Category="Boss Pattern|Events")
	FECBossPatternEvent OnPatternPrepared;

	/** 50%에 도달하여 스크림/폭발 연출을 요청할 때 발생합니다. */
	UPROPERTY(BlueprintAssignable, Category="Boss Pattern|Events")
	FECBossPatternEvent OnDetonationRequested;

	/** 준비된 모든 장판이 폭발하거나 제거됐을 때 발생합니다. */
	UPROPERTY(BlueprintAssignable, Category="Boss Pattern|Events")
	FECBossPatternEvent OnPatternFinished;

protected:
	/** 이 체력 비율 이하에서 장판을 한 번 생성합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Threshold",
		meta=(ClampMin="0.0", ClampMax="1.0"))
	float PrepareHealthRatio = 0.75f;

	/** 이 체력 비율 이하에서 스크림과 순차 폭발을 요청합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Threshold",
		meta=(ClampMin="0.0", ClampMax="1.0"))
	float DetonationHealthRatio = 0.5f;

	/** 체력이 이 비율 이상으로 회복되면 재도전으로 보고 패턴 상태를 초기화합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Threshold",
		meta=(EditCondition="bResetWhenHealthRestored", ClampMin="0.0", ClampMax="1.0"))
	float ResetHealthRatio = 0.99f;

	/** 보스 체력이 최대치 근처로 복구될 때 패턴을 다시 사용할 수 있게 초기화합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Threshold")
	bool bResetWhenHealthRestored = true;

	/** 생성할 장판 액터 클래스입니다. 데칼 머터리얼과 폭발 연출은 이 클래스의 BP 자식에서 지정합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Spawn")
	TSubclassOf<AECBossGroundHazard> HazardClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Spawn", meta=(ClampMin="0"))
	int32 MinHazardCount = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Spawn", meta=(ClampMin="0"))
	int32 MaxHazardCount = 7;

	/** 타겟 중심에서 장판이 생성될 최소 거리입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Spawn", meta=(ClampMin="0.0", Units="cm"))
	float MinSpawnDistance = 150.0f;

	/** 타겟 중심에서 장판이 생성될 최대 거리입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Spawn", meta=(ClampMin="0.0", Units="cm"))
	float MaxSpawnDistance = 850.0f;

	/** 장판 중심끼리 유지할 최소 간격입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Spawn", meta=(ClampMin="0.0", Units="cm"))
	float MinHazardSpacing = 350.0f;

	/** 각 장판 위치를 찾을 최대 시도 횟수입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Spawn", meta=(ClampMin="1"))
	int32 MaxLocationAttemptsPerHazard = 16;

	/** 실제 데미지와 넉백을 판정할 수평 반경입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Hazard", meta=(ClampMin="0.0", Units="cm"))
	float HazardRadius = 250.0f;

	/**
	 * 데칼 투영 박스의 반경입니다.
	 * 머터리얼 텍스처에 투명 여백이 있어 표시된 원이 작다면 데미지 반경과 별도로 이 값을 키웁니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Hazard", meta=(ClampMin="0.0", Units="cm"))
	float HazardDecalRadius = 250.0f;

	/** 장판 크기는 고정한 채 투명도만 0에서 1로 변화하는 시간입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Hazard", meta=(ClampMin="0.0", Units="s"))
	float HazardFadeInDuration = 0.5f;

	/** 장판과 타겟 사이에서 허용할 최대 높이 차이입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Hazard", meta=(ClampMin="0.0", Units="cm"))
	float MaxTargetHeightDifference = 180.0f;

	/** 첫 번째 이후 장판이 폭발하는 간격입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Sequence", meta=(ClampMin="0.0", Units="s"))
	float ExplosionInterval = 0.3f;

	/** 폭발 순서를 매번 섞습니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Sequence")
	bool bRandomizeExplosionOrder = true;

	/** 50% 요청 뒤 AnimNotify가 없어도 자동으로 폭발 시퀀스를 시작합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Sequence")
	bool bAutoStartDetonation = true;

	/** 자동 시작을 사용할 때 스크림 시작부터 첫 장판 시퀀스까지 기다리는 시간입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Sequence",
		meta=(EditCondition="bAutoStartDetonation", ClampMin="0.0", Units="s"))
	float AutoDetonationStartDelay = 1.0f;

	/** 장판 폭발에 적용할 데미지 GameplayEffect입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Damage", meta=(ClampMin="0.0"))
	float DamageEffectLevel = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Damage")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Damage", meta=(ClampMin="0.0"))
	float StaminaDamage = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Damage")
	FGameplayTag WeaponTypeTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Damage")
	FGameplayTag AttackTypeTag;

	/** 폭발에 적중한 캐릭터에게 장판 중심 바깥 방향으로 넉백을 적용합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Knockback")
	bool bApplyHazardKnockback = true;

	/** 넉백 반응에 전달할 초기 수평 속도입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Knockback",
		meta=(EditCondition="bApplyHazardKnockback", ClampMin="0.0", Units="cm/s"))
	float HazardKnockbackSpeed = 900.0f;

	/** 랜덤 후보 위치 위쪽으로 올려 지면 Trace를 시작하는 거리입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Ground Trace", meta=(ClampMin="0.0", Units="cm"))
	float GroundTraceUpDistance = 500.0f;

	/** Trace 시작점에서 아래로 검사하는 거리입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Ground Trace", meta=(ClampMin="0.0", Units="cm"))
	float GroundTraceDownDistance = 1200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Ground Trace")
	TEnumAsByte<ECollisionChannel> GroundTraceChannel = ECC_Visibility;

	/** 지면으로 인정할 최소 Normal.Z입니다. 1은 완전한 평면입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Ground Trace",
		meta=(ClampMin="-1.0", ClampMax="1.0"))
	float MinGroundNormalZ = 0.65f;

	/** Z-fighting 방지를 위해 표면 노멀 방향으로 장판을 띄우는 거리입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Ground Trace", meta=(Units="cm"))
	float SurfaceOffset = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss Pattern|Debug")
	bool bDrawDebug = false;

private:
	UFUNCTION()
	void HandleHealthChanged(
		UECHealthComponent* ChangedHealthComponent,
		float OldValue,
		float NewValue,
		AActor* DamageInstigator);

	UFUNCTION()
	void HandleOwnerDeath(AActor* OwningActor);

	void PrepareHazards();
	void RequestDetonation();
	void CompletePattern();
	void CleanupHazards();
	void HandleHazardFinished(AECBossGroundHazard* Hazard);
	AActor* ResolvePatternTarget() const;
	bool TryFindHazardTransform(
		const FVector& Center,
		AActor* TargetActor,
		const TArray<FVector>& AcceptedLocations,
		FTransform& OutTransform) const;
	FGameplayEffectSpecHandle MakeDamageSpec(AECBossGroundHazard* Hazard) const;

	UPROPERTY(Transient)
	TObjectPtr<UECHealthComponent> HealthComponent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AECBossGroundHazard>> PreparedHazards;

	UPROPERTY(Transient)
	TObjectPtr<AActor> PatternTargetOverride;

	TWeakObjectPtr<UAbilitySystemComponent> SourceASC;
	FTimerHandle AutoDetonationTimerHandle;
	int32 PendingHazardCount = 0;
	bool bHazardsPrepared = false;
	bool bDetonationRequested = false;
	bool bDetonationStarted = false;
	bool bPatternFinished = false;
	bool bIsResetting = false;
};
