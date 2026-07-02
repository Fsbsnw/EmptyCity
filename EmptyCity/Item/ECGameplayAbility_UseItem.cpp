#include "ECGameplayAbility_UseItem.h"
#include "AbilitySystemComponent.h"
#include "ECGameplayTags.h"
#include "GameplayEffect.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Inventory/Fragment/InventoryFragment_Consumable.h"

UECGameplayAbility_UseItem::UECGameplayAbility_UseItem()
{
	// 입력이 아니라 GameplayEvent로 발동되는 어빌리티이므로, 트리거를 직접 등록한다.
	// QuickBar 번호 키, 인벤토리 UI 등 모든 진입점이 이 태그로 이벤트를 보내면 이 어빌리티가 발동된다.
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = ECGameplayTags::GameplayEvent_Item_Use;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UECGameplayAbility_UseItem::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// GameplayEvent로만 발동하는 어빌리티이므로 payload가 없으면 사용할 대상이 없다.
	if (!TriggerEventData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%hs] : TriggerEventData가 없어 사용을 중단합니다."), __FUNCTION__);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// "어떤 아이템을 쓸지"는 입력 경로와 무관하게 payload(OptionalObject)로 받는다. (단일 GA 통합의 핵심)
	const UECInventoryItemInstance* Item = Cast<UECInventoryItemInstance>(TriggerEventData->OptionalObject);
	UE_LOG(LogTemp, Warning, TEXT("[%hs] : 아이템 사용 발동: %s"), __FUNCTION__, Item ? *Item->GetName() : TEXT("null"));

	// 소비형 Fragment가 없거나 적용할 GE가 지정되지 않았으면 사용 가능한 아이템이 아니다.
	const UInventoryFragment_Consumable* Consumable = Item ? Item->FindFragmentByClass<UInventoryFragment_Consumable>() : nullptr;
	if (!Consumable || !Consumable->EffectToApply)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%hs] : 소비형 Fragment 또는 적용할 GE가 없어 사용을 중단합니다."), __FUNCTION__);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 인벤토리에서 먼저 소모를 시도한다. 남은 수량이 부족하면(false) 효과를 적용하지 않는다. ("남아있다면" 조건)
	const AECPlayerController* PC = GetECPlayerControllerFromActorInfo();
	UECInventoryManagerComponent* Inventory = PC->GetInventoryManagerComponent();
	if (!Inventory->ConsumeItemsByDefinition(Item->ItemDef, Consumable->ConsumeCount))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%hs] : 아이템이 부족하여 소모에 실패했습니다. (필요: %d)"), __FUNCTION__, Consumable->ConsumeCount);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("[%hs] : 아이템 %d개 소모 완료. GE를 적용합니다."), __FUNCTION__, Consumable->ConsumeCount);

	// Fragment가 지정한 GE를 사용자에게 적용한다. 회복 종류(HP/Stamina)·회복량은 전적으로 GE 에셋이 결정한다.
	if (UECAbilitySystemComponent* ASC = GetECAbilitySystemComponentFromActorInfo())
	{
		const FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
		const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Consumable->EffectToApply, GetAbilityLevel(), Ctx);
		if (Spec.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			UE_LOG(LogTemp, Warning, TEXT("[%hs] : GE 적용 완료: %s"), __FUNCTION__, *Consumable->EffectToApply->GetName());
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
