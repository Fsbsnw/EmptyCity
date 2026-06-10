#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "InteractionOption.h"
#include "IInteractableTarget.generated.h"

struct FInteractionQuery;

/**
 * FInteractionOptionBuilder
 *
 * 상호작용 대상이 자신의 옵션들을 "수집(gather)"할 때 사용하는 헬퍼 클래스입니다.
 * 대상은 AddInteractionOption()만 호출하면 되고, 이 빌더가 옵션의 InteractableTarget 필드를
 * 해당 대상(Scope)으로 자동 설정해 줍니다. 덕분에 대상은 자기 자신을 일일이 채울 필요가 없습니다.
 */
class FInteractionOptionBuilder
{
	
public:
	FInteractionOptionBuilder(TScriptInterface<IInteractableTarget> InterfaceTargetScope, TArray<FInteractionOption>& InteractOptions)
		: Scope(InterfaceTargetScope)
		, Options(InteractOptions)
	{
	}

	/** 옵션 하나를 목록에 추가합니다. 추가하면서 InteractableTarget 필드를 Scope로 자동 채웁니다. */
	void AddInteractionOption(const FInteractionOption& Option)
	{
		FInteractionOption& OptionEntry = Options.Add_GetRef(Option);
		OptionEntry.InteractableTarget = Scope;
	}

private:
	/** 옵션을 제공하는 대상입니다. */
	TScriptInterface<IInteractableTarget> Scope;
	
	/** 옵션이 쌓이는 외부 배열입니다. */
	TArray<FInteractionOption>& Options;
};

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInteractableTarget : public UInterface
{
	GENERATED_BODY()
};

/**
 * IInteractableTarget
 *
 * "상호작용 가능한 물체"가 구현해야 하는 인터페이스입니다.
 * 문, 상자, NPC 등 플레이어가 상호작용할 수 있는 액터/컴포넌트가 이 인터페이스를 구현합니다.
 */
class IInteractableTarget
{
	GENERATED_BODY()

public:
	/**
	 * 이 대상이 제공하는 상호작용 옵션들을 수집합니다.
	 * InteractQuery(누가 요청하는지)를 보고, 제공할 옵션을 OptionBuilder.AddInteractionOption()으로 추가합니다.
	 * 순수 가상 함수이므로 모든 구현 클래스가 반드시 정의해야 합니다.
	 */
	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& OptionBuilder) = 0;

	/**
	 * 상호작용 어빌리티가 발동될 때 보내는 게임플레이 이벤트 데이터를 대상이 원하는 대로 수정할 수 있게 합니다.
	 * 기본 구현은 비어 있으며, 필요한 대상만 선택적으로 오버라이드합니다.
	 */
	virtual void CustomizeInteractionEventData(const FGameplayTag& InteractionEventTag, FGameplayEventData& InOutEventData) { }
};
