// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/ECProgressionSubsystem.h"

#include "Core/Setting/ECProgressionSettings.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "Inventory/Fragment/InventoryFragment_Unlockable.h"

void UECProgressionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 1. 프로젝트 설정에 등록된 기본 해금 태그들을 가져옵니다.
	const UECProgressionSettings* Settings = GetDefault<UECProgressionSettings>();
	if (Settings)
	{
		// 2. 기본 해금 태그들을 현재 해금 목록에 전부 집어넣습니다.
		UnlockedTags.AppendTags(Settings->DefaultUnlockedTags);
	}
}

bool UECProgressionSubsystem::IsContentUnlocked(FGameplayTag Tag) const
{
	return UnlockedTags.HasTagExact(Tag);
}

bool UECProgressionSubsystem::IsItemUnlocked(TSubclassOf<UECInventoryItemDefinition> ItemClass) const
{
	if (!ItemClass) return false;

	// 1. 아이템에서 해금 프래그먼트를 찾는다
	const UECInventoryItemDefinition* ItemDef = GetDefault<UECInventoryItemDefinition>(ItemClass);
	const UInventoryFragment_Unlockable* Fragment = Cast<UInventoryFragment_Unlockable>(ItemDef->FindFragmentByClass(UInventoryFragment_Unlockable::StaticClass()));

	// 2. 프래그먼트가 존재하고, 태그가 설정되어 있다면? -> 해금된 태그 목록과 대조
	if (Fragment && Fragment->RequiredUnlockTag.IsValid())
	{
		return IsContentUnlocked(Fragment->RequiredUnlockTag);
	}

	// 3. 프래그먼트가 없거나 태그가 비어있다면? -> 기본 해금 아이템이므로 true
	return true;
}

void UECProgressionSubsystem::UnlockContent(FGameplayTag UnlockTag)
{
	// 이미 해금된 지역/아이템이 아니라면 새로 추가합니다.
	if (!UnlockedTags.HasTagExact(UnlockTag))
	{
		UnlockedTags.AddTag(UnlockTag);
        
		// 아직 플레이어가 연출을 보지 않았기 때문에 대기열에도 넣어둡니다.
		PendingRevealTags.AddTag(UnlockTag); 
	}
}

void UECProgressionSubsystem::LockContent(FGameplayTag LockTag)
{
	UnlockedTags.RemoveTag(LockTag);
	PendingRevealTags.RemoveTag(LockTag);
}

bool UECProgressionSubsystem::CheckAndConsumeReveal(FGameplayTag Tag)
{
	// 대기열에 이 태그가 들어있다면?
	if (PendingRevealTags.HasTagExact(Tag))
	{
		// 확인했단 의미로 대기열에서 지워버립니다. 
		PendingRevealTags.RemoveTag(Tag);
        
		return true; // 연출을 실행하라고 알려줍니다.
	}
	return false; // 이미 연출을 봤거나, 아예 해금이 안 된 상태
}
