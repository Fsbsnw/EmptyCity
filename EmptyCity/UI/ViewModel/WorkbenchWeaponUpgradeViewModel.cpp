#include "UI/ViewModel/WorkbenchWeaponUpgradeViewModel.h"

#include "Character/Player/Controller/ECPlayerController.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Data/Workbench/ECWorkbenchUpgradeDataAsset.h"
#include "Inventory/ECInventoryManagerComponent.h"

namespace
{
	const FName WeaponWeightValueId(TEXT("Weight"));
	const FName WeaponAttackPowerValueId(TEXT("AttackPower"));
	const FName WeaponAttackSpeedValueId(TEXT("AttackSpeedMultiplier"));

	bool FindNonNegativeValue(
		const FECWorkbenchUpgradeLevelData* LevelData,
		FName ValueId,
		float& OutValue)
	{
		if (!LevelData)
		{
			return false;
		}

		const FECWorkbenchUpgradeValue* Value = LevelData->Values.FindByPredicate(
			[ValueId](const FECWorkbenchUpgradeValue& Candidate)
			{
				return Candidate.ValueId == ValueId;
			});
		if (!Value || Value->Value < 0.0f)
		{
			return false;
		}

		OutValue = Value->Value;
		return true;
	}

	FText AsNumber(float Value)
	{
		FNumberFormattingOptions Formatting;
		Formatting.MinimumFractionalDigits = 0;
		Formatting.MaximumFractionalDigits = 2;
		return FText::AsNumber(Value, &Formatting);
	}
}

void UWorkbenchWeaponUpgradeViewModel::Initialize(AActor* ContextActor)
{
	UECViewModelBase::BindCallbacksToDependencies(ContextActor);

	if (AECPlayerState* PreviousPlayerState = PlayerState.Get())
	{
		PreviousPlayerState->OnWorkbenchUpgradeLevelChanged.RemoveAll(this);
	}
	if (UECInventoryManagerComponent* PreviousInventory = InventoryComponent.Get())
	{
		PreviousInventory->OnInventoryUpdated.RemoveAll(this);
	}

	PlayerState = Cast<AECPlayerState>(GetPlayerState());
	const AECPlayerController* PlayerController = Cast<AECPlayerController>(GetPlayerController());
	InventoryComponent = PlayerController ? PlayerController->GetInventoryManagerComponent() : nullptr;

	if (AECPlayerState* CurrentPlayerState = PlayerState.Get())
	{
		CurrentPlayerState->OnWorkbenchUpgradeLevelChanged.AddUObject(
			this,
			&ThisClass::HandleUpgradeLevelChanged);
	}
	if (UECInventoryManagerComponent* Inventory = InventoryComponent.Get())
	{
		Inventory->OnInventoryUpdated.AddUObject(this, &ThisClass::HandleInventoryUpdated);
	}

	Refresh();
}

void UWorkbenchWeaponUpgradeViewModel::Refresh()
{
	const AECPlayerState* CurrentPlayerState = PlayerState.Get();
	const UECWorkbenchUpgradeDataAsset* UpgradeData =
		CurrentPlayerState ? CurrentPlayerState->GetWorkbenchUpgradeData() : nullptr;
	const FECWorkbenchUpgradeDefinition* Definition = UpgradeData
		? UpgradeData->FindUpgradeDefinition(EECWorkbenchUpgradeType::Weapon)
		: nullptr;

	if (!CurrentPlayerState || !Definition)
	{
		NextRequirements.Reset();
		SetDisplayName(NSLOCTEXT("Workbench", "WeaponDisplayName", "무기"));
		SetCurrentLevel(1);
		SetMaxLevel(0);
		SetCurrentWeight(0.0f);
		SetNextWeight(0.0f);
		SetCurrentAttackPower(0.0f);
		SetNextAttackPower(0.0f);
		SetCurrentAttackSpeedMultiplier(0.0f);
		SetNextAttackSpeedMultiplier(0.0f);
		SetHasNextLevel(false);
		SetHasValidData(false);
		SetCanUpgrade(false);
		OnPreviewRefreshed.Broadcast();
		return;
	}

	const int32 NewCurrentLevel = CurrentPlayerState->GetWorkbenchUpgradeLevel(
		EECWorkbenchUpgradeType::Weapon);
	const int32 NewMaxLevel = UpgradeData->GetMaxConfiguredLevel(EECWorkbenchUpgradeType::Weapon);
	const FECWorkbenchUpgradeLevelData* CurrentLevelData = UpgradeData->FindLevelData(
		EECWorkbenchUpgradeType::Weapon,
		NewCurrentLevel);
	const FECWorkbenchUpgradeLevelData* NextLevelData = UpgradeData->FindNextLevelData(
		EECWorkbenchUpgradeType::Weapon,
		NewCurrentLevel);

	float NewCurrentWeight = 0.0f;
	float NewNextWeight = 0.0f;
	float NewCurrentAttackPower = 0.0f;
	float NewNextAttackPower = 0.0f;
	float NewCurrentAttackSpeed = 0.0f;
	float NewNextAttackSpeed = 0.0f;
	const bool bFoundCurrentWeight = FindNonNegativeValue(
		CurrentLevelData, WeaponWeightValueId, NewCurrentWeight);
	const bool bFoundNextWeight = FindNonNegativeValue(
		NextLevelData, WeaponWeightValueId, NewNextWeight);
	const bool bFoundCurrentAttackPower = FindNonNegativeValue(
		CurrentLevelData, WeaponAttackPowerValueId, NewCurrentAttackPower);
	const bool bFoundNextAttackPower = FindNonNegativeValue(
		NextLevelData, WeaponAttackPowerValueId, NewNextAttackPower);
	const bool bFoundCurrentAttackSpeed = FindNonNegativeValue(
		CurrentLevelData, WeaponAttackSpeedValueId, NewCurrentAttackSpeed)
		&& NewCurrentAttackSpeed > UE_KINDA_SMALL_NUMBER;
	const bool bFoundNextAttackSpeed = FindNonNegativeValue(
		NextLevelData, WeaponAttackSpeedValueId, NewNextAttackSpeed)
		&& NewNextAttackSpeed > UE_KINDA_SMALL_NUMBER;

	NextRequirements = NextLevelData
		? NextLevelData->RequiredItems
		: TArray<FECInventoryItemRequirement>();

	const bool bCurrentValuesValid = bFoundCurrentWeight
		&& bFoundCurrentAttackPower
		&& bFoundCurrentAttackSpeed;
	const bool bNextValuesValid = bFoundNextWeight
		&& bFoundNextAttackPower
		&& bFoundNextAttackSpeed;

	SetDisplayName(Definition->DisplayName.IsEmpty()
		? NSLOCTEXT("Workbench", "WeaponDisplayName", "무기")
		: Definition->DisplayName);
	SetCurrentLevel(NewCurrentLevel);
	SetMaxLevel(NewMaxLevel);
	SetCurrentWeight(NewCurrentWeight);
	SetNextWeight(bFoundNextWeight ? NewNextWeight : NewCurrentWeight);
	SetCurrentAttackPower(NewCurrentAttackPower);
	SetNextAttackPower(bFoundNextAttackPower ? NewNextAttackPower : NewCurrentAttackPower);
	SetCurrentAttackSpeedMultiplier(NewCurrentAttackSpeed);
	SetNextAttackSpeedMultiplier(
		bFoundNextAttackSpeed ? NewNextAttackSpeed : NewCurrentAttackSpeed);
	SetHasNextLevel(NextLevelData && bNextValuesValid);
	SetHasValidData(NewMaxLevel > 0 && bCurrentValuesValid);
	SetCanUpgrade(
		bCurrentValuesValid
		&& NextLevelData
		&& bNextValuesValid
		&& CurrentPlayerState->CanExecuteWorkbenchUpgrade(EECWorkbenchUpgradeType::Weapon));
	OnPreviewRefreshed.Broadcast();
}

bool UWorkbenchWeaponUpgradeViewModel::ExecuteUpgrade()
{
	AECPlayerState* CurrentPlayerState = PlayerState.Get();
	return CurrentPlayerState
		&& CurrentPlayerState->ExecuteWorkbenchUpgrade(EECWorkbenchUpgradeType::Weapon);
}

void UWorkbenchWeaponUpgradeViewModel::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(DisplayName);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentLevel);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(MaxLevel);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentWeight);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(NextWeight);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentAttackPower);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(NextAttackPower);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentAttackSpeedMultiplier);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(NextAttackSpeedMultiplier);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(bHasNextLevel);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(bHasValidData);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(bCanUpgrade);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentWeightText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNextWeightText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentAttackPowerText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNextAttackPowerText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentAttackSpeedText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNextAttackSpeedText);
}

void UWorkbenchWeaponUpgradeViewModel::BeginDestroy()
{
	if (AECPlayerState* CurrentPlayerState = PlayerState.Get())
	{
		CurrentPlayerState->OnWorkbenchUpgradeLevelChanged.RemoveAll(this);
	}
	if (UECInventoryManagerComponent* Inventory = InventoryComponent.Get())
	{
		Inventory->OnInventoryUpdated.RemoveAll(this);
	}

	PlayerState.Reset();
	InventoryComponent.Reset();
	Super::BeginDestroy();
}

void UWorkbenchWeaponUpgradeViewModel::HandleUpgradeLevelChanged(
	EECWorkbenchUpgradeType ChangedType,
	int32 NewLevel)
{
	if (ChangedType == EECWorkbenchUpgradeType::Weapon)
	{
		Refresh();
	}
}

void UWorkbenchWeaponUpgradeViewModel::HandleInventoryUpdated()
{
	Refresh();
}

FText UWorkbenchWeaponUpgradeViewModel::GetCurrentWeightText() const
{
	return AsNumber(CurrentWeight);
}

FText UWorkbenchWeaponUpgradeViewModel::GetNextWeightText() const
{
	return AsNumber(NextWeight);
}

FText UWorkbenchWeaponUpgradeViewModel::GetCurrentAttackPowerText() const
{
	return AsNumber(CurrentAttackPower);
}

FText UWorkbenchWeaponUpgradeViewModel::GetNextAttackPowerText() const
{
	return AsNumber(NextAttackPower);
}

FText UWorkbenchWeaponUpgradeViewModel::GetCurrentAttackSpeedText() const
{
	return AsNumber(CurrentAttackSpeedMultiplier);
}

FText UWorkbenchWeaponUpgradeViewModel::GetNextAttackSpeedText() const
{
	return AsNumber(NextAttackSpeedMultiplier);
}

void UWorkbenchWeaponUpgradeViewModel::SetDisplayName(const FText& NewValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(DisplayName, NewValue);
}

void UWorkbenchWeaponUpgradeViewModel::SetCurrentLevel(int32 NewValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentLevel, NewValue);
}

void UWorkbenchWeaponUpgradeViewModel::SetMaxLevel(int32 NewValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxLevel, NewValue);
}

void UWorkbenchWeaponUpgradeViewModel::SetCurrentWeight(float NewValue)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(CurrentWeight, NewValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentWeightText);
	}
}

void UWorkbenchWeaponUpgradeViewModel::SetNextWeight(float NewValue)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(NextWeight, NewValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNextWeightText);
	}
}

void UWorkbenchWeaponUpgradeViewModel::SetCurrentAttackPower(float NewValue)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(CurrentAttackPower, NewValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentAttackPowerText);
	}
}

void UWorkbenchWeaponUpgradeViewModel::SetNextAttackPower(float NewValue)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(NextAttackPower, NewValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNextAttackPowerText);
	}
}

void UWorkbenchWeaponUpgradeViewModel::SetCurrentAttackSpeedMultiplier(float NewValue)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(CurrentAttackSpeedMultiplier, NewValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentAttackSpeedText);
	}
}

void UWorkbenchWeaponUpgradeViewModel::SetNextAttackSpeedMultiplier(float NewValue)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(NextAttackSpeedMultiplier, NewValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNextAttackSpeedText);
	}
}

void UWorkbenchWeaponUpgradeViewModel::SetHasNextLevel(bool bNewValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasNextLevel, bNewValue);
}

void UWorkbenchWeaponUpgradeViewModel::SetHasValidData(bool bNewValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasValidData, bNewValue);
}

void UWorkbenchWeaponUpgradeViewModel::SetCanUpgrade(bool bNewValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bCanUpgrade, bNewValue);
}
