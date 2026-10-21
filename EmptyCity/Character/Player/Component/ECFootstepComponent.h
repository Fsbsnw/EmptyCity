#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ECFootstepComponent.generated.h"

class ACharacter;

UCLASS()
class EMPTYCITY_API UECFootstepComponent : public UActorComponent
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// ActorComponent Interface
// ─────────────────────────────────────────────────────────────
public:
	/** 발소리 처리를 위한 틱을 활성화합니다. */
	UECFootstepComponent();

	/** 소유 캐릭터를 캐싱하여 발소리 처리를 준비합니다. */
	virtual void BeginPlay() override;

	/** 지상 이동 거리를 누적하여 발소리 재생 시점을 판정합니다. */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


// ─────────────────────────────────────────────────────────────
// Footstep Method
// ─────────────────────────────────────────────────────────────
private:
	/** 발밑 표면에 대응하는 발소리를 재생합니다. */
	void PlayFootstep();


// ─────────────────────────────────────────────────────────────
// Footstep Variable
// ─────────────────────────────────────────────────────────────
protected:
	/** 한 번의 발소리를 재생하기 위해 이동해야 하는 거리입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|발소리", meta = (ClampMin = "1.0"))
	float StepDistance = 140.0f;


// ─────────────────────────────────────────────────────────────
// Cached Variable
// ─────────────────────────────────────────────────────────────
private:
	/** 이 컴포넌트를 소유한 캐릭터입니다. */
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	/** 마지막 발소리 이후 누적한 지상 이동 거리입니다. */
	float AccumulatedDistance = 0.0f;
};
