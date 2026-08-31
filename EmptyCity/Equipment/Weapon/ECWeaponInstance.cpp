#include "ECWeaponInstance.h"

#include "ECGameplayTags.h"
#include "ECWeaponDefinition.h"
#include "WeaponTraceComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Cosmetic/ECPawnComponent_CharacterParts.h"
#include "Character/Player/ECPlayer.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/Weapon/ECWeaponTableRow.h"
#include "Data/Weapon/ECWeaponSettings.h"
#include "Equipment/ECEquipmentDefinition.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Inventory/Fragment/InventoryFragment_EquippableItem.h"

void UECWeaponInstance::OnEquipped()
{
	Super::OnEquipped();

	FindWeaponTraceComponent();

	// 코스메틱 태그를 먼저 수집한 뒤 AnimLayer를 결정해야 올바른 레이어가 선택됩니다.
	DetermineCosmeticTags();
	ActivateAnimLayerAndPlayPairedAnim();
}

void UECWeaponInstance::OnUnequipped()
{
	ClearActiveAttack();
	WeaponTraceComponent.Reset();

	Super::OnUnequipped();
}

const FECWeaponTableRow* UECWeaponInstance::GetWeaponData() const
{
	const UECInventoryItemInstance* ItemInstance = Cast<UECInventoryItemInstance>(Instigator);

	if (!ItemInstance)
	{
		return nullptr;
	}

	const UInventoryFragment_EquippableItem* EquipFragment = ItemInstance->FindFragmentByClass<UInventoryFragment_EquippableItem>();

	if (!EquipFragment || !EquipFragment->EquipmentDefinition)
	{
		return nullptr;
	}

	const UECEquipmentDefinition* EquipmentDefinition = GetDefault<UECEquipmentDefinition>(EquipFragment->EquipmentDefinition);
	const UECWeaponDefinition* WeaponDefinition = Cast<UECWeaponDefinition>(EquipmentDefinition);

	return WeaponDefinition ? WeaponDefinition->GetWeaponData() : nullptr;
}

const FECWeaponAttackDefinition* UECWeaponInstance::SetActiveAttack(const FGameplayTag& AttackTag)
{
	UWeaponTraceComponent* TraceComponent = WeaponTraceComponent.Get();

	if (!IsValid(TraceComponent))
	{
		return nullptr;
	}

	return TraceComponent->SetActiveAttack(AttackTag);
}

void UECWeaponInstance::ClearActiveAttack()
{
	if (UWeaponTraceComponent* TraceComponent = WeaponTraceComponent.Get(); IsValid(TraceComponent))
	{
		TraceComponent->ClearActiveAttack();
	}
}

void UECWeaponInstance::BeginTrace()
{
	if (UWeaponTraceComponent* TraceComponent = WeaponTraceComponent.Get(); IsValid(TraceComponent))
	{
		TraceComponent->BeginTrace();
	}
}

void UECWeaponInstance::EndTrace()
{
	if (UWeaponTraceComponent* TraceComponent = WeaponTraceComponent.Get(); IsValid(TraceComponent))
	{
		TraceComponent->EndTrace();
	}
}

void UECWeaponInstance::FindWeaponTraceComponent()
{
	WeaponTraceComponent.Reset();

	for (AActor* SpawnedActor : GetSpawnedActors())
	{
		if (!IsValid(SpawnedActor))
		{
			continue;
		}

		if (UWeaponTraceComponent* Trace = SpawnedActor->FindComponentByClass<UWeaponTraceComponent>())
		{
			WeaponTraceComponent = Trace;
			Trace->SetOwningWeaponInstance(this);
			break;
		}
	}
}

void UECWeaponInstance::DetermineCosmeticTags()
{
	CosmeticAnimStyle.Reset();
	APawn* OwningPawn = GetPawn();
	if (!IsValid(OwningPawn))
	{
		return;
	}

	// 루트 태그 없이 전체 태그를 요청해 현재 장착된 모든 코스메틱 정보를 CosmeticAnimStyle에 저장합니다.
	if (UECPawnComponent_CharacterParts* CosmeticComponent = OwningPawn->FindComponentByClass<UECPawnComponent_CharacterParts>())
	{
		CosmeticAnimStyle = CosmeticComponent->GetCombinedTags(FGameplayTag());
	}
}

void UECWeaponInstance::ActivateAnimLayerAndPlayPairedAnim()
{
	// 장착(bEquipped=true) 상태 기준으로 최적 AnimLayer를 선택합니다.
	UClass* AnimClass = PickBestAnimLayer(true, CosmeticAnimStyle);
	AECPlayer* Player = Cast<AECPlayer>(GetPawn());
	USkeletalMeshComponent* FirstPersonMesh = IsValid(Player) ? Player->GetFirstPersonMesh() : nullptr;
	if (!AnimClass || !IsValid(FirstPersonMesh))
	{
		return;
	}

	// 선택된 AnimLayer를 Mesh에 동적으로 링크해 무기별 애니메이션 로직을 활성화합니다.
	FirstPersonMesh->LinkAnimClassLayers(AnimClass);

	// AnimLayer 링크 이후에 몽타주를 재생해야 올바른 슬롯에서 출력됩니다.
	if (WeaponEquipmentMontage)
	{
		if (UAnimInstance* AnimInstance = FirstPersonMesh->GetAnimInstance())
		{
			AnimInstance->Montage_Play(WeaponEquipmentMontage);
		}
	}
}

TSubclassOf<UAnimInstance> UECWeaponInstance::PickBestAnimLayer(bool bEquipped, const FGameplayTagContainer& CosmeticTags) const
{
	// bEquipped 여부에 따라 참조할 AnimSet을 결정하고, CosmeticTags와 매칭되는 최적 레이어를 반환합니다.
	const FECAnimLayerSelectionSet& SetToQuery = (bEquipped ? EquippedAnimSet : UnequippedAnimSet);
	return SetToQuery.SelectBestLayer(CosmeticTags);
}
