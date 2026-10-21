#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeathVisualComponent.generated.h"

class UCurveFloat;
class UMaterialInstanceDynamic;
class UNiagaraComponent;
class UNiagaraSystem;

/**
 * 적의 사망 애니메이션이 끝난 뒤 디졸브 머터리얼과 나이아가라 연출을 재생합니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EMPTYCITY_API UDeathVisualComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDeathVisualComponent();

	/** 대기 시간과 디졸브 연출을 재생하고 완료 시 InFinishedDelegate를 실행합니다. */
	void PlayDeathVisual(FSimpleDelegate InFinishedDelegate);

private:
	void StartDissolve();
	void UpdateDissolve();
	void CompleteDissolve();
	void TryFinishDeathVisual();
	void FinishDeathVisual();

	UFUNCTION()
	void HandleNiagaraSystemFinished(UNiagaraComponent* FinishedComponent);

	void HandleNiagaraCompletionTimeout();

protected:
	/** 사망 애니메이션 종료 후 디졸브가 시작되기까지의 대기 시간입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Dissolve", meta = (ClampMin = "0.0", Units = "s"))
	float DissolveStartDelay = 2.0f;

	/** 디졸브가 처음부터 끝까지 진행되는 시간입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Dissolve", meta = (ClampMin = "0.0", Units = "s"))
	float DissolveDuration = 2.0f;

	/** 적 머터리얼에서 사용하는 디졸브 Scalar Parameter 이름입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Dissolve")
	FName DissolveParameterName = TEXT("Dissolve");

	/** 디졸브 시작 시 머터리얼에 전달할 값입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Dissolve")
	float DissolveStartValue = 0.0f;

	/** 디졸브 완료 시 머터리얼에 전달할 값입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Dissolve")
	float DissolveEndValue = 2.0f;

	/** 디졸브 진행 곡선입니다. 지정하지 않으면 선형으로 진행합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Dissolve")
	TObjectPtr<UCurveFloat> DissolveCurve;

	/** 디졸브 시작 시 소유 캐릭터의 메시를 기준으로 부착할 나이아가라 이펙트입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Dissolve")
	TObjectPtr<UNiagaraSystem> DeathDissolveVFX;

	/** 머터리얼 디졸브 값과 함께 갱신할 Niagara User Float Parameter입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Dissolve")
	FName NiagaraDissolveParameterName = TEXT("User.Dissolve");

	/** 머터리얼 디졸브 완료 후 Niagara 종료를 기다리는 최대 시간입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Dissolve", meta = (ClampMin = "0.0", Units = "s"))
	float NiagaraCompletionTimeout = 2.0f;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DissolveMaterialInstances;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> DeathDissolveComponent;

	FSimpleDelegate FinishedDelegate;
	FTimerHandle DissolveStartTimerHandle;
	FTimerHandle DissolveUpdateTimerHandle;
	FTimerHandle NiagaraCompletionTimerHandle;
	float DissolveStartedAt = 0.0f;
	bool bIsPlaying = false;
	bool bDissolveFinished = false;
	bool bNiagaraFinished = true;
};
