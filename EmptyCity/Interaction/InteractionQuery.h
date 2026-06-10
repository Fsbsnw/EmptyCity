#pragma once

#include "CoreMinimal.h"
#include "InteractionQuery.generated.h"

/**
 * FInteractionQuery
 *
 * "누가 상호작용을 요청하는가"에 대한 정보를 담는 구조체입니다.
 * 상호작용 대상(IInteractableTarget)에게 "이런 캐릭터가 너와 상호작용을 시도 중이다"라고
 * 전달할 때 사용됩니다. 이 정보를 보고 대상은 자신이 어떤 상호작용 옵션을 제공할지 결정합니다.
 *
 * 예) 문(IInteractableTarget)은 RequestingAvatar가 열쇠를 가지고 있는지 등을 검사할 수 있습니다.
 */
USTRUCT(BlueprintType)
struct FInteractionQuery
{
	GENERATED_BODY()

public:
	/** 상호작용을 요청하는 폰(캐릭터)입니다. 보통 플레이어 캐릭터가 들어갑니다. */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> RequestingAvatar;

	/** 상호작용을 요청하는 컨트롤러입니다. RequestingAvatar의 실제 소유자와 달라도 됩니다(따로 지정 가능). */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AController> RequestingController;

	/** 상호작용에 필요한 추가 데이터를 자유롭게 담을 수 있는 범용 UObject 슬롯입니다. */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> OptionalObjectData;

};
