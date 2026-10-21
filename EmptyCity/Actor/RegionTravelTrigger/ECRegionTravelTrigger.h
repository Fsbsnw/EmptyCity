#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ECRegionTravelTrigger.generated.h"

class UBoxComponent;
class UPrimitiveComponent;

/**
 * 플레이어가 영역에 들어오면 지정한 지역으로 이동시키는 범용 트리거입니다.
 * 기본값은 광신도 본거지 문 앞에서 9일차 이후 보스 전투 구역으로 이동하는 설정입니다.
 */
UCLASS(Blueprintable)
class EMPTYCITY_API AECRegionTravelTrigger : public AActor
{
	GENERATED_BODY()

public:
	AECRegionTravelTrigger();

	/** 현재 날짜와 현재 지역을 기준으로 이 트리거를 사용할 수 있는지 반환합니다. */
	UFUNCTION(BlueprintPure, Category="Region|Travel")
	bool IsTravelAvailable() const;

private:
	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

private:
	/** 문 앞에서 플레이어 진입을 감지할 영역입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Region|Travel", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> TriggerBox;

	/** 이동할 지역입니다. 해당 태그의 AECRegionTeleportPoint가 월드에 배치되어 있어야 합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Region|Travel", meta=(AllowPrivateAccess="true"))
	FGameplayTag TargetRegionTag;

	/** 이 지역에 있을 때만 트리거가 동작합니다. 비워 두면 현재 지역을 검사하지 않습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Region|Travel", meta=(AllowPrivateAccess="true"))
	FGameplayTag RequiredOriginRegionTag;

	/** 이 날짜부터 트리거가 동작합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Region|Travel", meta=(AllowPrivateAccess="true", ClampMin="1"))
	int32 MinimumDay = 9;
};
