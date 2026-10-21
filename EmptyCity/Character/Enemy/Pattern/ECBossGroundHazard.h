#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "ECBossGroundHazard.generated.h"

class UAbilitySystemComponent;
class UDecalComponent;
class UMaterialInterface;
class UNiagaraSystem;
class USceneComponent;
class USoundBase;

DECLARE_MULTICAST_DELEGATE_OneParam(FECBossGroundHazardFinished, class AECBossGroundHazard*);

/**
 * 보스 패턴이 미리 배치하는 단일 지면 장판입니다.
 * 최종 크기의 데칼을 페이드인으로 표시하고, 무장된 뒤 자체 타이머로 폭발과 데미지를 처리합니다.
 */
UCLASS(Blueprintable)
class EMPTYCITY_API AECBossGroundHazard : public AActor
{
	GENERATED_BODY()

public:
	AECBossGroundHazard();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** SpawnDeferred 중 호출하여 데미지 범위와 데칼 투영 크기, 페이드 시간을 설정합니다. */
	void ConfigureVisual(
		float InDamageRadius,
		float InDecalRadius,
		float InFadeInDuration,
		float InMaxTargetHeightDifference);

	/** SpawnDeferred 중 호출하여 이 패턴의 넉백 정책을 설정합니다. */
	void ConfigureKnockback(bool bInApplyKnockback, float InKnockbackSpeed);

	/** 서버에서 데미지 Spec을 전달하고 Delay 뒤 폭발하도록 무장합니다. */
	void ArmForDetonation(
		UAbilitySystemComponent* InSourceASC,
		const FGameplayEffectSpecHandle& InDamageSpec,
		float Delay);

	/** 무장 대기 시간을 건너뛰고 즉시 폭발시킵니다. */
	UFUNCTION(BlueprintCallable, Category="Boss Pattern|Ground Hazard")
	void Detonate();

	float GetDamageRadius() const { return DamageRadius; }
	bool HasDetonated() const { return bHasDetonated; }

	FECBossGroundHazardFinished OnFinished;

protected:
	/** 바닥에 투영할 원형 경고 데칼 머터리얼입니다. Decal Lifetime Opacity를 Opacity에 반영해야 페이드됩니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ground Hazard|Visual")
	TObjectPtr<UMaterialInterface> WarningDecalMaterial;

	/** 데칼이 바닥을 투영하는 깊이입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ground Hazard|Visual", meta=(ClampMin="0.1", Units="cm"))
	float DecalProjectionDepth = 24.0f;

	/** 폭발할 때 재생할 Niagara 이펙트입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ground Hazard|Explosion")
	TObjectPtr<UNiagaraSystem> ExplosionVFX;

	/** 폭발할 때 재생할 사운드입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ground Hazard|Explosion")
	TObjectPtr<USoundBase> ExplosionSFX;

	/** 폭발 연출 재생 후 액터가 제거되기까지 유지하는 시간입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ground Hazard|Explosion", meta=(ClampMin="0.0", Units="s"))
	float PostExplosionLifeTime = 1.0f;

private:
	UPROPERTY(VisibleAnywhere, Category="Ground Hazard")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category="Ground Hazard")
	TObjectPtr<UDecalComponent> WarningDecal;

	UPROPERTY(ReplicatedUsing=OnRep_VisualConfig)
	float DamageRadius = 250.0f;

	UPROPERTY(ReplicatedUsing=OnRep_VisualConfig)
	float DecalRadius = 250.0f;

	UPROPERTY(ReplicatedUsing=OnRep_VisualConfig)
	float FadeInDuration = 0.5f;

	UPROPERTY(Replicated)
	float MaxTargetHeightDifference = 180.0f;

	UFUNCTION()
	void OnRep_VisualConfig();

	UFUNCTION()
	void OnRep_Detonated();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayExplosion();

	void ApplyVisualConfig();
	void StartFadeIn();
	void ApplyDamageToOverlappingTargets();
	void SendKnockbackEvent(AActor* TargetActor) const;
	bool CanDamageTarget(AActor* TargetActor) const;
	void BroadcastFinishedOnce();

	TWeakObjectPtr<UAbilitySystemComponent> SourceASC;
	FGameplayEffectSpecHandle DamageSpec;
	FTimerHandle DetonationTimerHandle;
	bool bApplyKnockback = true;
	float KnockbackSpeed = 900.0f;

	UPROPERTY(ReplicatedUsing=OnRep_Detonated)
	bool bHasDetonated = false;

	bool bFinishedBroadcast = false;
};
