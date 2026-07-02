#include "ECInventoryItemDefinition.h"

const UECInventoryItemFragment* UECInventoryItemDefinition::FindFragmentByClass(TSubclassOf<UECInventoryItemFragment> FragmentClass) const
{
	if (FragmentClass)
	{
		for (UECInventoryItemFragment* Fragment : Fragments)
		{
			// IsA를 사용해 파생 클래스도 매칭되도록 한다.
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}