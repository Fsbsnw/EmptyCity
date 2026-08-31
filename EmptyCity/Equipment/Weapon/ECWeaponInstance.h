#pragma once

#include "CoreMinimal.h"
#include "Cosmetic/CosmeticAnimationTypes.h"
#include "Equipment/ECEquipmentInstance.h"
#include "Equipment/Weapon/WeaponTraceComponent.h"
#include "ECWeaponInstance.generated.h"

struct FECWeaponTableRow;
class UAnimMontage;


/**
 * 무기 전용 EquipmentInstance 기반 클래스입니다.
 *
 * EquipmentInstance를 상속하여 무기 고유 기능을 추가합니다.
 * - EquippedAnimSet / UnequippedAnimSet: 장착/비장착 상태에 따라 올바른 Animation Layer를
 *   링크하기 위한 설정값입니다. AnimInstance가 이 값을 읽어 Linked Anim Layer를 동적으로 교체합니다.
 * - 원거리/근거리 무기별 세부 로직은 파생 클래스에서 구현합니다.
 */
UCLASS()
class EMPTYCITY_API UECWeaponInstance : public UECEquipmentInstance
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// Equip / Unequip
// ─────────────────────────────────────────────────────────────
public:
	/** 장착 이벤트입니다. Cosmetic 태그를 수집하고 AnimLayer 연결 및 장착 몽타주를 재생합니다. */
	virtual void OnEquipped() override;
	virtual void OnUnequipped() override;
	
	
	
// ─────────────────────────────────────────────────────────────
// Weapon
// ─────────────────────────────────────────────────────────────
public:
	/** 현재 무기에 연결된 데이터 테이블 행을 반환합니다. */
	const FECWeaponTableRow* GetWeaponData() const;


// ─────────────────────────────────────────────────────────────
// Attack
// ─────────────────────────────────────────────────────────────
public:
	/** AttackTag에 대응하는 공격을 WeaponTraceComponent의 현재 공격으로 선택합니다. */
	const FECWeaponAttackDefinition* SetActiveAttack(const FGameplayTag& AttackTag);

	/** 현재 공격 및 판정 상태를 정리합니다. */
	void ClearActiveAttack();

	/** 현재 공격 정의를 사용해 무기 판정을 시작합니다. */
	void BeginTrace();

	/** 진행 중인 무기 판정을 종료합니다. */
	void EndTrace();

private:
	void FindWeaponTraceComponent();


// ─────────────────────────────────────────────────────────────
// Animation
// ─────────────────────────────────────────────────────────────
public:
	/** Pawn의 CharacterParts 컴포넌트에서 현재 장착된 코스메틱 태그를 읽어 CosmeticAnimStyle에 저장합니다. */
	void DetermineCosmeticTags();

	/** CosmeticAnimStyle을 기준으로 최적 AnimLayer를 선택해 Mesh에 링크하고, 장착 몽타주를 재생합니다. */
	void ActivateAnimLayerAndPlayPairedAnim();

	/** bEquipped와 CosmeticTags를 기준으로 Equipped/Unequipped AnimSet 중 가장 적합한 AnimLayer 클래스를 반환합니다. */
	TSubclassOf<UAnimInstance> PickBestAnimLayer(bool bEquipped, const FGameplayTagContainer& CosmeticTags) const;


// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
public:
	/** Pawn의 코스메틱 상태를 반영하는 태그입니다. PickBestAnimLayer 호출 시 규칙 매칭에 사용됩니다. */
	FGameplayTagContainer CosmeticAnimStyle;

	/** 무기를 장착한 상태에서 사용할 Animation Layer 선택 설정입니다. */
	UPROPERTY(EditAnywhere, Category = "변수")
	FECAnimLayerSelectionSet EquippedAnimSet;

	/** 무기를 들지 않은(Unequipped) 상태에서 사용할 Animation Layer 선택 설정입니다. */
	UPROPERTY(EditAnywhere, Category = "변수")
	FECAnimLayerSelectionSet UnequippedAnimSet;

	/** 무기 장착 시 출력할 애니메이션 몽타주입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수")
	TObjectPtr<UAnimMontage> WeaponEquipmentMontage;
	
	/** 가드 시작/루프/종료 섹션을 가진 몽타주입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "변수|Animation|Guard")
	TObjectPtr<UAnimMontage> GuardMontage;

	/** 스폰된 무기 Actor가 소유하는 공격 판정 컴포넌트입니다. */
	UPROPERTY(Transient)
	TWeakObjectPtr<UWeaponTraceComponent> WeaponTraceComponent;

};
