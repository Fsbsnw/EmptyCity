#include "ECEquipmentManagerComponent.h"
#include "AbilitySystemGlobals.h"
#include "ECEquipmentDefinition.h"
#include "ECEquipmentInstance.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"

UECEquipmentInstance* FEquipmentList::AddEntry(TSubclassOf<UECEquipmentDefinition> EquipmentDefinition)
{
	UECEquipmentInstance* Result = nullptr;
	check(EquipmentDefinition != nullptr);
	check(OwnerComponent);
	check(OwnerComponent->GetOwner()->HasAuthority());

	// CDO에서 InstanceType과 ActorsToSpawn을 읽는다. (런타임 인스턴스 생성 전에 에셋 정보를 참조)
	const UECEquipmentDefinition* EquipmentCDO = GetDefault<UECEquipmentDefinition>(EquipmentDefinition);

	TSubclassOf<UECEquipmentInstance> InstanceType = EquipmentCDO->InstanceType;
	if (!InstanceType)
	{
		// Blueprint에서 InstanceType을 지정하지 않은 경우 기본 클래스로 폴백한다.
		InstanceType = UECEquipmentInstance::StaticClass();
	}

	FAppliedEquipmentEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.EquipmentDefinition = EquipmentDefinition;
	// Pawn을 Outer로 지정해 GetPawn()이 올바른 소유자를 반환할 수 있게 한다.
	NewEntry.Instance = NewObject<UECEquipmentInstance>(OwnerComponent->GetOwner(), InstanceType);
	Result = NewEntry.Instance;
	
	UECAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	check(ASC);
	{
		for (const TObjectPtr<UECAbilitySet> AbilitySet : EquipmentCDO->AbilitySetsToGrant)
		{
			AbilitySet->GiveToAbilitySystem(ASC, &NewEntry.GrantedHandles, Result);
		}
	}

	Result->SpawnEquipmentActors(EquipmentCDO->ActorsToSpawn);
	return Result;
}

void FEquipmentList::RemoveEntry(UECEquipmentInstance* Instance)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FAppliedEquipmentEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			UECAbilitySystemComponent* ASC = GetAbilitySystemComponent();
			check(ASC);
			{
				Entry.GrantedHandles.TakeFromAbilitySystem(ASC);
			}
			
			// 스폰된 Actor를 먼저 정리한 뒤 엔트리를 제거한다.
			Instance->DestroyEquipmentActors();
			EntryIt.RemoveCurrent();
		}
	}
}

UECAbilitySystemComponent* FEquipmentList::GetAbilitySystemComponent() const
{
	check(OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner();
	
	return Cast<UECAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor));
}

UECEquipmentManagerComponent::UECEquipmentManagerComponent(const FObjectInitializer& Initializer) : Super(Initializer)
                                                                                                      ,EquipmentList(this)
{
}

UECEquipmentInstance* UECEquipmentManagerComponent::EquipItem(TSubclassOf<UECEquipmentDefinition> EquipmentDefinition)
{
	UECEquipmentInstance* Result = nullptr;
	if (EquipmentDefinition)
	{
		Result = EquipmentList.AddEntry(EquipmentDefinition);
		if (Result)
		{
			// Actor 스폰이 완료된 뒤 장착 이벤트를 발행한다.
			Result->OnEquipped();
		}
	}
	return Result;
}

void UECEquipmentManagerComponent::UnequipItem(UECEquipmentInstance* ItemInstance)
{
	if (ItemInstance)
	{
		// 해제 이벤트를 먼저 발행한 뒤 Actor를 파괴하고 엔트리를 제거한다.
		ItemInstance->OnUnequipped();
		EquipmentList.RemoveEntry(ItemInstance);
	}
}

TArray<UECEquipmentInstance*> UECEquipmentManagerComponent::GetEquipmentInstancesOfType(TSubclassOf<UECEquipmentInstance> InstanceType) const
{
	TArray<UECEquipmentInstance*> Results;
	
	for (const FAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (UECEquipmentInstance* Instance = Entry.Instance)
		{
			if (Instance->IsA(InstanceType))
			{
				Results.Add(Instance);
			}
		}
	}
	return Results;
}