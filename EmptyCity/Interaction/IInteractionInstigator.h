#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "InteractionOption.h"
#include "IInteractionInstigator.generated.h"

struct FInteractionQuery;

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInteractionInstigator : public UInterface
{
	GENERATED_BODY()
};

/**
 * IInteractionInstigator
 *
 * 상호작용을 "시작하는 쪽"(보통 플레이어)이 선택적으로 구현하는 인터페이스입니다.
 * 상호작용 옵션이 여러 개 발견되었을 때, 그 중 어느 것을 실행할지 직접 결정하는
 * 중재자 역할을 추가할 수 있게 해줍니다.
 *
 * 예) 어떤 게임은 옵션이 여러 개일 때 메뉴를 띄워 플레이어가 직접 고르게 합니다.
 *     이 인터페이스를 구현하면 그렇게 여러 후보 중 하나를 골라낼 수 있습니다.
 */
class IInteractionInstigator
{
	GENERATED_BODY()

public:
	/** 결정이 필요한 InteractOptions가 두 개 이상일 때 호출됩니다. 그 중 최선의 옵션 하나를 반환합니다. */
	virtual FInteractionOption ChooseBestInteractionOption(const FInteractionQuery& InteractQuery, const TArray<FInteractionOption>& InteractOptions) = 0;
};
