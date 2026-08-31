#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "ECAnimNotifyState_GameplayTagWindow.generated.h"

/**
 * Notify 구간 동안 애니메이션 소유자의 AbilitySystemComponent에 GameplayTag를 추가합니다.
 * 동일 태그를 사용하는 구간이 겹쳐도 안전하도록 loose tag 개수를 증감합니다.
 */
UCLASS(meta = (DisplayName = "EC Gameplay Tag Window"))
class EMPTYCITY_API UECAnimNotifyState_GameplayTagWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UECAnimNotifyState_GameplayTagWindow(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	/** Notify 구간 동안 소유자의 ASC에 유지할 태그입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Tag", meta = (Categories = "Status"))
	FGameplayTag WindowTag;
};
