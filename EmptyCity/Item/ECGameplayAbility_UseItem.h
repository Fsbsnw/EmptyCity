#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/ECGameplayAbility.h"
#include "ECGameplayAbility_UseItem.generated.h"

/**
 * 인벤토리 아이템 "사용"을 처리하는 범용 어빌리티입니다.
 *
 * [설계 의도 — 진입점 무관 단일 GA]
 * - QuickBar 번호 키, 인벤토리 UI 등 어떤 경로로든 "Event.Item.Use" GameplayEvent에
 *   사용할 ItemInstance를 payload(OptionalObject)로 실어 보내면 이 어빌리티가 발동됩니다.
 * - 무엇을 적용/회복할지는 이 어빌리티가 아니라 아이템의 Fragment(UInventoryFragment_Consumable)가 결정합니다.
 *   따라서 HP 포션, Stamina 포션처럼 종류가 늘어도 이 클래스는 수정할 필요가 없습니다.
 *
 * [발동 방식]
 * 입력이 아니라 GameplayEvent로 트리거됩니다. 생성자에서 AbilityTriggers를 등록하므로,
 * 플레이어 ASC에 한 번 부여해 두면 별도 입력 바인딩 없이 이벤트만으로 발동됩니다.
 */
UCLASS()
class EMPTYCITY_API UECGameplayAbility_UseItem : public UECGameplayAbility
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// GameplayAbility Interface
// ─────────────────────────────────────────────────────────────
public:
	UECGameplayAbility_UseItem();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

};
