// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/AI/BTDecorator/BTDecorator_CheckGameplayTag.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"

UBTDecorator_CheckGameplayTag::UBTDecorator_CheckGameplayTag()
{
	NodeName = "Check Gameplay Tag";
    
	// Delegate 사용 설정
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;

	// 독립된 Decorator 노드 인스턴스가 필요한 경우 수정
	// bCreateNodeInstance = true;
}

bool UBTDecorator_CheckGameplayTag::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	// ASC를 가져와서 태그가 있는지 없는지 체크
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerComp.GetAIOwner()->GetPawn());
	if (!ASC) return false;

	return ASC->HasMatchingGameplayTag(TagToCheck);
}

void UBTDecorator_CheckGameplayTag::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerComp.GetAIOwner()->GetPawn());
	if (ASC)
	{
		// 설정한 태그 변화를 감지합니다.
		ASC->RegisterGameplayTagEvent(TagToCheck, EGameplayTagEventType::NewOrRemoved)
		   .AddUObject(this, &UBTDecorator_CheckGameplayTag::OnGameplayTagChanged, TWeakObjectPtr<UBehaviorTreeComponent>(&OwnerComp));
	}
}

void UBTDecorator_CheckGameplayTag::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerComp.GetAIOwner()->GetPawn());
	if (ASC)
	{
		// 설정한 태그에 등록된 델리게이트를 제거합니다.
		ASC->RegisterGameplayTagEvent(TagToCheck, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
	}
}

void UBTDecorator_CheckGameplayTag::OnGameplayTagChanged(const FGameplayTag Tag, int32 NewCount, TWeakObjectPtr<UBehaviorTreeComponent> OwnerComp)
{
	if (OwnerComp.IsValid())
	{
		// 설정한 태그가 변화될 때 데코레이터를 다시 실행시킵니다.
		OwnerComp->RequestExecution(this);
	}
}