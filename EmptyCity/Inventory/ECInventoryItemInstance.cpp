#include "ECInventoryItemInstance.h"
#include "ECInventoryItemDefinition.h"

const UECInventoryItemFragment* UECInventoryItemInstance::FindFragmentByClass(TSubclassOf<UECInventoryItemFragment> FragmentClass) const
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		// Instance는 런타임 상태만 보유하고, Fragment 데이터는 공유 에셋인 Definition의 CDO에서 읽는다.
		return GetDefault<UECInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}

void UECInventoryItemInstance::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void UECInventoryItemInstance::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

int32 UECInventoryItemInstance::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

bool UECInventoryItemInstance::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}
