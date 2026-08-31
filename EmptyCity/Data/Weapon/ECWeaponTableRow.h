#pragma once

#include "CoreMinimal.h"
#include "ECWeaponTableRow.generated.h"

/**
 * 무기 한 종류의 고정 데이터를 정의하는 데이터 테이블 행입니다.
 *
 * 무기의 이름, 무게, 공격력, 공격속도 및 공격별 스태미나 소모량처럼
 * 무기 종류에 따라 달라지는 정적 데이터를 저장합니다.
 *
 * 무기의 구매 및 해금 여부와 같은 플레이어별 런타임 상태는
 * 이 구조체가 아닌 ProgressionSubsystem에서 별도로 관리합니다.
 */
USTRUCT(BlueprintType)
struct FECWeaponTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** UI에 표시할 무기의 한글 이름입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "NameKo"))
	FText KoreanName;

	/** UI에 표시할 무기의 영어 이름입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "NameEn"))
	FText EnglishName;

	/**
	 * 무기의 무게입니다.
	 * 장비 무게 제한, 이동속도 또는 스태미나 계산 등에 사용할 수 있습니다.
	 * 단위는 kg을 기준으로 합니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "Weight"), meta = (ClampMin = "0.0", Units = "kg"))
	float Weight = 0.0f;

	/**
	 * 무기 자체의 기본 공격력입니다.
	 * 최종 피해 계산 시 플레이어의 힘 스탯에 더해집니다.
	 *
	 * 최종 피해 = (플레이어 힘 + 무기 공격력) × 공격 배율
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "AtkPower"), meta = (ClampMin = "0.0"))
	float AttackPower = 0.0f;

	/**
	 * 공격 몽타주에 적용할 재생속도 배율입니다.
	 *
	 * 1.0이면 몽타주 원본 속도로 재생되고,
	 * 1.2이면 20% 빠르게, 0.8이면 20% 느리게 재생됩니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "AtkSpeed"), meta = (ClampMin = "0.01"))
	float AttackSpeedMultiplier = 1.0f;

	/**
	 * 약공격을 한 번 사용할 때 소모하는 스태미나입니다.
	 * 공격 어빌리티의 Cost 검사와 실제 스태미나 차감에 사용됩니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "StaminaCost_Light"), meta = (ClampMin = "0.0"))
	float LightAttackStaminaCost = 0.0f;

	/**
	 * 강공격을 한 번 사용할 때 소모하는 스태미나입니다.
	 * 공격 어빌리티의 Cost 검사와 실제 스태미나 차감에 사용됩니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "StaminaCost_Heavy"), meta = (ClampMin = "0.0"))
	float HeavyAttackStaminaCost = 0.0f;
};