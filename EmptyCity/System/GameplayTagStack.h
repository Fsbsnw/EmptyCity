#pragma once

#include "GameplayTagContainer.h"
#include "GameplayTagStack.generated.h"

/**
 * 게임플레이 태그 스택들을 담는 컨테이너입니다.
 * "어떤 태그를 몇 개 쌓아 두었는지"를 관리하며, 아이템 수량/누적 효과 등에 사용됩니다.
 *
 * [싱글플레이 변환]
 * 원본 Lyra는 네트워크 복제를 위해 FFastArraySerializer 기반의 배열(Stacks)과
 * 조회 가속용 맵(TagToCountMap)을 이중으로 관리했습니다.
 * 이 프로젝트는 싱글플레이만 다루므로 복제 의존을 제거하고, 태그→개수 맵 하나로만 관리합니다.
 */
USTRUCT(BlueprintType)
struct FGameplayTagStackContainer
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Add / Remove
// ─────────────────────────────────────────────────────────────
public:
	/** 지정한 태그의 스택을 그 개수만큼 늘립니다. 같은 태그가 이미 있으면 합치고, 없으면 새로 만듭니다. (StackCount가 1 미만이면 무시) */
	void AddStack(FGameplayTag Tag, int32 StackCount);

	/** 지정한 태그의 스택을 그 개수만큼 줄입니다. 0 이하가 되면 해당 태그를 목록에서 제거합니다. (StackCount가 1 미만이면 무시) */
	void RemoveStack(FGameplayTag Tag, int32 StackCount);

	
// ─────────────────────────────────────────────────────────────
// Query
// ─────────────────────────────────────────────────────────────
public:
	/** 지정한 태그의 현재 스택 개수를 반환합니다. (태그가 없으면 0) */
	int32 GetStackCount(FGameplayTag Tag) const
	{
		return TagToCountMap.FindRef(Tag);
	}

	/** 지정한 태그의 스택이 하나라도 있으면 true를 반환합니다. */
	bool ContainsTag(FGameplayTag Tag) const
	{
		return TagToCountMap.Contains(Tag);
	}

// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
private:
	/** 태그별 스택 개수를 담는 맵입니다. 스택 정보를 보관하는 유일한 저장소입니다. */
	UPROPERTY()
	TMap<FGameplayTag, int32> TagToCountMap;
	
};
