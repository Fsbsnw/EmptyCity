#include "InventoryFragment_SetStats.h"
#include "Inventory/ECInventoryItemInstance.h"

void UInventoryFragment_SetStats::OnInstanceCreated(UECInventoryItemInstance* Instance) const
{
	// 설계도에 정의된 초기 스탯들을 인스턴스의 StatTags에 그대로 쌓아 시작 상태를 만든다.
	for (const auto& KVP : InitialItemStats)
	{
		Instance->AddStatTagStack(KVP.Key, KVP.Value);
	}
}

int32 UInventoryFragment_SetStats::GetItemStatByTag(FGameplayTag Tag) const
{
	// 정의된 태그면 그 값을, 없으면 0을 반환한다.
	if (const int32* StatPtr = InitialItemStats.Find(Tag))
	{
		return *StatPtr;
	}

	return 0;
}
