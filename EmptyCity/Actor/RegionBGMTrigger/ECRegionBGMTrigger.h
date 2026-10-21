#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "ECRegionBGMTrigger.generated.h"

class UBoxComponent;
class UPrimitiveComponent;

/** 지역 BGM을 어떤 방식으로 전환할지 지정합니다. */
UENUM(BlueprintType)
enum class EECBGMTransitionType : uint8
{
	Immediate UMETA(DisplayName = "즉시 전환"),
	FadeIn UMETA(DisplayName = "페이드 인"),
	CrossFade UMETA(DisplayName = "크로스페이드")
};

/** 플레이어가 영역에 진입하면 DT_Sound의 지정 행을 BGM으로 재생합니다. */
UCLASS(Blueprintable)
class EMPTYCITY_API AECRegionBGMTrigger : public AActor
{
	GENERATED_BODY()

public:
	AECRegionBGMTrigger();

	/** 에디터에 설정된 DT_Sound 행을 즉시 재생 요청합니다. */
	UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
	void PlayConfiguredBGM();

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
	/** 플레이어 진입을 감지할 영역입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sound|BGM", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> TriggerBox;

	/** 재생할 DT_Sound의 행입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound|BGM", meta = (AllowPrivateAccess = "true", RowType = "/Script/EmptyCity.SoundTableRow"))
	FDataTableRowHandle BGMRow;

	/** 기존 BGM에서 새 BGM으로 전환하는 방식입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound|BGM", meta = (AllowPrivateAccess = "true"))
	EECBGMTransitionType TransitionType = EECBGMTransitionType::CrossFade;

	/** 페이드 인 또는 크로스페이드에 사용할 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound|BGM", meta = (AllowPrivateAccess = "true", EditCondition = "TransitionType != EECBGMTransitionType::Immediate", ClampMin = "0.0", Units = "s"))
	float TransitionDuration = 2.0f;

	/** 켜면 이 트리거는 게임 실행 중 한 번만 BGM을 재생합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound|BGM", meta = (AllowPrivateAccess = "true"))
	bool bTriggerOnce = false;

	bool bHasTriggered = false;
};
