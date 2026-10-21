#include "ECWeaponInstance.h"

#include "ECGameplayTags.h"
#include "ECWeaponDefinition.h"
#include "WeaponTraceComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Cosmetic/ECPawnComponent_CharacterParts.h"
#include "Character/Player/ECPlayer.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/Weapon/ECWeaponTableRow.h"
#include "Data/Weapon/ECWeaponSettings.h"
#include "Equipment/ECEquipmentDefinition.h"
#include "Inventory/ECInventoryItemInstance.h"
#include "Inventory/Fragment/InventoryFragment_EquippableItem.h"

void UECWeaponInstance::OnEquipped()
{
	Super::OnEquipped();

	FindWeaponTraceComponent();
	BindWeaponUpgradeEvents();
	RefreshWeaponVisual();

	// 코스메틱 태그를 먼저 수집한 뒤 AnimLayer를 결정해야 올바른 레이어가 선택됩니다.
	DetermineCosmeticTags();
	ActivateAnimLayerAndPlayPairedAnim();
}

void UECWeaponInstance::OnUnequipped()
{
	UnbindWeaponUpgradeEvents();
	ClearActiveAttack();
	WeaponTraceComponent.Reset();
	WeaponVisualMeshComponent.Reset();

	Super::OnUnequipped();
}

const FECWeaponTableRow* UECWeaponInstance::GetWeaponData() const
{
	const UECWeaponDefinition* WeaponDefinition = GetWeaponDefinition();
	return WeaponDefinition ? WeaponDefinition->GetWeaponData() : nullptr;
}

const UECWeaponDefinition* UECWeaponInstance::GetWeaponDefinition() const
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
	return Cast<UECWeaponDefinition>(EquipmentDefinition);
}

float UECWeaponInstance::GetEffectiveAttackPower() const
{
	const FECWeaponTableRow* WeaponData = GetWeaponData();
	const float BaseAttackPower = WeaponData ? FMath::Max(0.0f, WeaponData->AttackPower) : 0.0f;
	const APawn* OwningPawn = GetPawn();
	const AECPlayerState* PlayerState = OwningPawn
		? OwningPawn->GetPlayerState<AECPlayerState>()
		: nullptr;
	float UpgradedAttackPower = 0.0f;
	return PlayerState
		&& PlayerState->GetWorkbenchUpgradeValue(
			EECWorkbenchUpgradeType::Weapon,
			TEXT("AttackPower"),
			UpgradedAttackPower)
		&& UpgradedAttackPower >= 0.0f
		? UpgradedAttackPower
		: BaseAttackPower;
}

float UECWeaponInstance::GetEffectiveAttackSpeedMultiplier() const
{
	const FECWeaponTableRow* WeaponData = GetWeaponData();
	const float BaseAttackSpeed = WeaponData
		? FMath::Max(UE_KINDA_SMALL_NUMBER, WeaponData->AttackSpeedMultiplier)
		: 1.0f;
	const APawn* OwningPawn = GetPawn();
	const AECPlayerState* PlayerState = OwningPawn
		? OwningPawn->GetPlayerState<AECPlayerState>()
		: nullptr;
	float UpgradedAttackSpeed = 0.0f;
	return PlayerState
		&& PlayerState->GetWorkbenchUpgradeValue(
			EECWorkbenchUpgradeType::Weapon,
			TEXT("AttackSpeedMultiplier"),
			UpgradedAttackSpeed)
		&& UpgradedAttackSpeed > UE_KINDA_SMALL_NUMBER
		? UpgradedAttackSpeed
		: BaseAttackSpeed;
}

bool UECWeaponInstance::GetParryImpactTransform(FTransform& OutTransform) const
{
	const UStaticMeshComponent* VisualMesh = WeaponVisualMeshComponent.Get();
	if (!IsValid(VisualMesh) || !IsValid(VisualMesh->GetStaticMesh()))
	{
		return false;
	}

	// 강화 단계마다 소켓의 전체 이름이 달라도 공통 접미사로 중앙 소켓을 찾습니다.
	static const FString MidSocketSuffix(TEXT("_Mid"));
	for (const FName SocketName : VisualMesh->GetAllSocketNames())
	{
		if (SocketName.ToString().EndsWith(MidSocketSuffix, ESearchCase::IgnoreCase))
		{
			OutTransform = VisualMesh->GetSocketTransform(SocketName, RTS_World);
			return true;
		}
	}

	// 중앙 소켓이 없는 임시 메시도 패링 이펙트 위치를 제공할 수 있도록 Bounds 중앙을 사용합니다.
	OutTransform = VisualMesh->GetComponentTransform();
	OutTransform.SetLocation(VisualMesh->Bounds.Origin);
	return true;
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

void UECWeaponInstance::BindWeaponUpgradeEvents()
{
	UnbindWeaponUpgradeEvents();

	APawn* OwningPawn = GetPawn();
	if (AECPlayerState* PlayerState = OwningPawn ? OwningPawn->GetPlayerState<AECPlayerState>() : nullptr)
	{
		PlayerState->OnWorkbenchUpgradeLevelChanged.AddUObject(
			this,
			&ThisClass::HandleWorkbenchUpgradeLevelChanged);
	}
}

void UECWeaponInstance::UnbindWeaponUpgradeEvents()
{
	APawn* OwningPawn = GetPawn();
	if (AECPlayerState* PlayerState = OwningPawn ? OwningPawn->GetPlayerState<AECPlayerState>() : nullptr)
	{
		PlayerState->OnWorkbenchUpgradeLevelChanged.RemoveAll(this);
	}
}

void UECWeaponInstance::RefreshWeaponVisual()
{
	const UECWeaponDefinition* WeaponDefinition = GetWeaponDefinition();
	APawn* OwningPawn = GetPawn();
	const AECPlayerState* PlayerState = OwningPawn
		? OwningPawn->GetPlayerState<AECPlayerState>()
		: nullptr;
	if (!WeaponDefinition || !PlayerState)
	{
		return;
	}

	UStaticMeshComponent* VisualMesh = FindOrCreateWeaponVisualMesh();
	if (!IsValid(VisualMesh))
	{
		return;
	}

	const int32 WeaponLevel = PlayerState->GetWorkbenchUpgradeLevel(EECWorkbenchUpgradeType::Weapon);
	VisualMesh->SetStaticMesh(WeaponDefinition->GetVisualMeshForUpgradeLevel(WeaponLevel));
	VisualMesh->SetCastShadow(false);
	VisualMesh->SetCastHiddenShadow(false);

	if (UWeaponTraceComponent* TraceComponent = WeaponTraceComponent.Get())
	{
		TraceComponent->SetTraceMesh(VisualMesh);
	}
}

UStaticMeshComponent* UECWeaponInstance::FindOrCreateWeaponVisualMesh()
{
	if (UStaticMeshComponent* ExistingVisualMesh = WeaponVisualMeshComponent.Get())
	{
		return ExistingVisualMesh;
	}

	UWeaponTraceComponent* TraceComponent = WeaponTraceComponent.Get();
	AActor* WeaponActor = IsValid(TraceComponent) ? TraceComponent->GetOwner() : nullptr;
	if (!IsValid(WeaponActor))
	{
		return nullptr;
	}

	UMeshComponent* PreviousVisualMesh = TraceComponent->GetTraceMesh();
	const FTransform PreviousRelativeTransform = IsValid(PreviousVisualMesh)
		? PreviousVisualMesh->GetRelativeTransform()
		: FTransform::Identity;

	UStaticMeshComponent* NewVisualMesh = NewObject<UStaticMeshComponent>(
		WeaponActor,
		TEXT("RuntimeWeaponVisualMesh"));
	if (!IsValid(NewVisualMesh))
	{
		return nullptr;
	}

	WeaponActor->AddInstanceComponent(NewVisualMesh);
	NewVisualMesh->SetMobility(EComponentMobility::Movable);
	NewVisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NewVisualMesh->SetGenerateOverlapEvents(false);
	NewVisualMesh->SetCastShadow(false);
	NewVisualMesh->SetCastHiddenShadow(false);
	NewVisualMesh->SetupAttachment(WeaponActor->GetRootComponent());
	NewVisualMesh->SetRelativeTransform(PreviousRelativeTransform);
	NewVisualMesh->RegisterComponent();

	if (IsValid(PreviousVisualMesh))
	{
		PreviousVisualMesh->SetCastShadow(false);
		PreviousVisualMesh->SetCastHiddenShadow(false);
		PreviousVisualMesh->SetVisibility(false, true);
		PreviousVisualMesh->SetHiddenInGame(true, true);
	}

	WeaponVisualMeshComponent = NewVisualMesh;
	return NewVisualMesh;
}

void UECWeaponInstance::HandleWorkbenchUpgradeLevelChanged(
	EECWorkbenchUpgradeType UpgradeType,
	int32 NewLevel)
{
	if (UpgradeType == EECWorkbenchUpgradeType::Weapon)
	{
		RefreshWeaponVisual();
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
