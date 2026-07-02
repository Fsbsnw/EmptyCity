#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ECEquipmentDefinition.generated.h"

class UECEquipmentInstance;
class UECAbilitySet;

/**
 * 장착 시 월드에 스폰할 Actor 한 개에 대한 설정값입니다.
 * 무기나 방어구처럼 눈에 보이는 Mesh가 있는 장비는, 캐릭터의 특정 소켓에 부착된 Actor로 표현됩니다.
 * (예: "오른손 소켓에 검 Actor를, 약간의 위치 보정을 주어 붙인다")
 */
USTRUCT()
struct FEquipmentActorToSpawn
{
	GENERATED_BODY()

	/** 스폰할 대상 Actor 클래스입니다. (예: 검, 총 등의 BP Actor) */
	UPROPERTY(EditAnywhere, Category = "Equipment")
	TSubclassOf<AActor> ActorToSpawn;

	/** 위 Actor를 캐릭터 Mesh의 어느 Bone 소켓에 부착할지 지정합니다. */
	UPROPERTY(EditAnywhere, Category = "Equipment")
	FName AttachSocket;

	/** 소켓 위치를 기준으로 추가로 적용할 오프셋(위치/회전/스케일) Transform입니다. */
	UPROPERTY(EditAnywhere, Category = "Equipment")
	FTransform AttachTransform;

};

/**
 * "장착 가능한 아이템이 실제로 어떻게 장착되는지"를 정의해 두는 데이터 에셋(설계도)입니다.
 * 런타임에 직접 수정하지 않고, 에디터에서 미리 값을 채워 두는 메타데이터 역할만 합니다.
 *
 * [InventoryItemDefinition과의 관계]
 * - InventoryItemDefinition : "인벤토리에 넣을 수 있는 아이템" 전반을 정의합니다.
 * - EquipmentDefinition(이 클래스) : 그 중 "장착 가능한 아이템"이 어떤 Actor/Ability로 장착되는지를 정의합니다.
 * - UECInventoryFragment_EquippableItem : 위 두 개념을 연결하는 다리 역할을 합니다. (아이템 → 장비 설계도)
 *
 * InstanceType을 파생 클래스(예: UECWeaponInstance)로 지정하면,
 * 장착 시 그 타입의 런타임 인스턴스가 생성되어 무기별 고유 로직을 담당합니다.
 */
UCLASS(Blueprintable)
class EMPTYCITY_API UECEquipmentDefinition : public UObject
{
	GENERATED_BODY()

public:
	/** 장착 시 생성할 EquipmentInstance의 클래스입니다. 지정하지 않으면 기본값 UECEquipmentInstance가 사용됩니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수")
	TSubclassOf<UECEquipmentInstance> InstanceType;

	/** 장착 시 월드에 스폰하여 캐릭터 소켓에 부착할 Actor 목록입니다. (검, 총처럼 눈에 보이는 부분) */
	UPROPERTY(EditDefaultsOnly, Category = "변수")
	TArray<FEquipmentActorToSpawn> ActorsToSpawn;

	/** 장착하는 동안 캐릭터에게 부여할 AbilitySet 목록입니다. (해제 시 함께 회수되는 공격/스킬 등) */
	UPROPERTY(EditDefaultsOnly, Category = "변수")
	TArray<TObjectPtr<UECAbilitySet>> AbilitySetsToGrant;

};
