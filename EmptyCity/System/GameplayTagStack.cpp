#include "GameplayTagStack.h"
#include "UObject/Stack.h"

void FGameplayTagStackContainer::AddStack(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("GameplayTagStack::AddStack : 잘못된 태그입니다."))
		return;
	}

	if (StackCount > 0)
	{
		// 기존 개수에 더하고, 없으면 새로 추가합니다.
		int32& Count = TagToCountMap.FindOrAdd(Tag);
		Count += StackCount;
	}
}

void FGameplayTagStackContainer::RemoveStack(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("GameplayTagStack::RemoveStack : 잘못된 태그입니다."))
		return;
	}

	if (StackCount > 0)
	{
		if (int32* Count = TagToCountMap.Find(Tag))
		{
			if (*Count <= StackCount)
			{
				// 남은 개수보다 많이 빼면 항목 자체를 제거합니다.
				TagToCountMap.Remove(Tag);
			}
			else
			{
				*Count -= StackCount;
			}
		}
	}
}
