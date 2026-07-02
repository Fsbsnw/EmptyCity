#pragma once

#include "CoreMinimal.h"
#include "System/GameplayTagStack.h"
#include "UObject/Object.h"
#include "ECInventoryItemInstance.generated.h"

class UECInventoryItemDefinition;
class UECInventoryItemFragment;
struct FGameplayTag;

/**
 * 인벤토리에 실제로 들어온 아이템 하나를 나타내는 런타임 인스턴스입니다.
 *
 * [Definition과의 차이]
 * - Definition(설계도) : 모든 같은 종류 아이템이 공유하는 고정 정보입니다.
 * - Instance(이 클래스) : 플레이어가 보유한 "그 한 개"를 가리킵니다.
 *
 * EquipmentManager를 통해 장착/해제됩니다.
 * 수량·탄약·내구도처럼 개별 아이템마다 달라지는 런타임 상태는 StatTags(태그→개수 맵)에 담습니다.
 */
UCLASS()
class EMPTYCITY_API UECInventoryItemInstance : public UObject
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Fragment Query
// ─────────────────────────────────────────────────────────────
public:
	/** ItemDef가 가진 Fragment 중 지정한 클래스에 해당하는 것을 찾아 반환합니다. (실제 검색은 공유 에셋인 Definition에 위임) */
	const UECInventoryItemFragment* FindFragmentByClass(TSubclassOf<UECInventoryItemFragment> FragmentClass) const;

	/** 위 함수의 템플릿 버전입니다. 캐스팅 없이 원하는 Fragment 타입으로 바로 받아올 수 있어 편리합니다. */
	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}

	
// ─────────────────────────────────────────────────────────────
// Stat Tags
// ─────────────────────────────────────────────────────────────
public:
	/** 지정한 태그의 스택을 그 개수만큼 늘립니다. 수량·탄약 같은 런타임 수치를 쌓는 데 사용합니다. (StackCount가 1 미만이면 무시) */
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	/** 지정한 태그의 스택을 그 개수만큼 줄입니다. 0 이하가 되면 해당 태그가 제거됩니다. (StackCount가 1 미만이면 무시) */
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	/** 지정한 태그의 현재 스택 개수를 반환합니다. (태그가 없으면 0) */
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	/** 지정한 태그의 스택이 하나라도 있으면 true를 반환합니다. */
	bool HasStatTag(FGameplayTag Tag) const;


// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
public:
	/** 이 인스턴스가 어떤 종류의 아이템인지 가리키는 설계도입니다. (공유 에셋이므로 직접 수정하지 않습니다) */
	UPROPERTY()
	TSubclassOf<UECInventoryItemDefinition> ItemDef;

	/** 이 아이템의 런타임 수치를 태그→개수로 담는 컨테이너입니다. (수량, 탄약, 누적 스탯 등) */
	UPROPERTY()
	FGameplayTagStackContainer StatTags;

};
