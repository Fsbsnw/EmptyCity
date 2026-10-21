#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ECCutsceneTrigger.generated.h"

class UBoxComponent;
class UPrimitiveComponent;

/** 플레이어가 지정된 영역에 진입하면 ECCutsceneSubsystem을 통해 컷씬을 재생합니다. */
UCLASS(Blueprintable)
class EMPTYCITY_API AECCutsceneTrigger : public AActor
{
	GENERATED_BODY()

public:
	AECCutsceneTrigger();

	/** 런타임에 생성된 액터를 컷씬 바인딩 대상으로 지정합니다. */
	UFUNCTION(BlueprintCallable, Category="Cutscene")
	void SetCutsceneContextActor(AActor* InContextActor);

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Cutscene", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> TriggerBox;

	/** DA_Cutscene에서 재생할 컷씬을 찾는 태그입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cutscene", meta=(AllowPrivateAccess="true", Categories="Cutscene"))
	FGameplayTag CutsceneTag;

	/**
	 * Level Sequence의 ContextActorBindingTag에 연결할 실제 월드 액터입니다.
	 * 지정하지 않으면 기존처럼 컨텍스트 없이 컷씬을 재생합니다.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Cutscene", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AActor> CutsceneContextActor;
};
