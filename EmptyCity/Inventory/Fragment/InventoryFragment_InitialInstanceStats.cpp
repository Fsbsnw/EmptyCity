#include "InventoryFragment_InitialInstanceStats.h"

#include "Inventory/ECInventoryItemInstance.h"

void UInventoryFragment_InitialInstanceStats::OnInstanceCreated(UECInventoryItemInstance* Instance) const
{
	if (!Instance)
	{
		return;
	}

	for (const TPair<FGameplayTag, int32>& InitialStat : InitialStats)
	{
		Instance->AddStatTagStack(InitialStat.Key, InitialStat.Value);
	}
}

int32 UInventoryFragment_InitialInstanceStats::GetItemStatByTag(FGameplayTag Tag) const
{
	const int32* StatValue = InitialStats.Find(Tag);
	return *StatValue;
}
