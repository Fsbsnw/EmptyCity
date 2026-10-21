#include "UI/ViewModel/WorkbenchStatUpgradeViewModel.h"

#include "Character/Player/Controller/ECPlayerController.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Data/Workbench/ECWorkbenchUpgradeDataAsset.h"
#include "Inventory/ECInventoryManagerComponent.h"

namespace ECWorkbenchStatValueIds
{
	const FName MaxHealth(TEXT("MaxHealth"));
	const FName MaxStamina(TEXT("MaxStamina"));
	const FName MaxInventoryWeight(TEXT("MaxInventoryWeight"));
}

namespace
{
	bool FindUpgradeValue(
		const FECWorkbenchUpgradeLevelData* LevelData,
		FName ValueId,
		float& OutValue)
	{
		if (!LevelData || ValueId.IsNone())
		{
			return false;
		}

		for (const FECWorkbenchUpgradeValue& UpgradeValue : LevelData->Values)
		{
			if (UpgradeValue.ValueId == ValueId)
			{
				OutValue = UpgradeValue.Value;
				return true;
			}
		}

		return false;
	}

	FText GetStatFallbackDisplayName(EECWorkbenchUpgradeType UpgradeType)
	{
		switch (UpgradeType)
		{
		case EECWorkbenchUpgradeType::MaxHealth:
			return NSLOCTEXT("Workbench", "MaxHealthDisplayName", "최대 체력");
		case EECWorkbenchUpgradeType::MaxStamina:
			return NSLOCTEXT("Workbench", "MaxStaminaDisplayName", "최대 스태미나");
		case EECWorkbenchUpgradeType::MaxInventoryWeight:
			return NSLOCTEXT("Workbench", "MaxInventoryWeightDisplayName", "최대 가방 무게");
		default:
			return FText::GetEmpty();
		}
	}
}

void UWorkbenchStatUpgradeViewModel::Initialize(
	AActor* ContextActor,
	EECWorkbenchUpgradeType InUpgradeType)
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

	SetUpgradeType(InUpgradeType);
}

void UWorkbenchStatUpgradeViewModel::SetUpgradeType(EECWorkbenchUpgradeType InUpgradeType)
{
	if (UpgradeType != InUpgradeType)
	{
		UpgradeType = InUpgradeType;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetUpgradeType);
	}

	Refresh();
}

void UWorkbenchStatUpgradeViewModel::Refresh()
{
	const AECPlayerState* CurrentPlayerState = PlayerState.Get();
	const UECWorkbenchUpgradeDataAsset* UpgradeData =
		CurrentPlayerState ? CurrentPlayerState->GetWorkbenchUpgradeData() : nullptr;
	const FECWorkbenchUpgradeDefinition* Definition =
		UpgradeData ? UpgradeData->FindUpgradeDefinition(UpgradeType) : nullptr;

	if (!CurrentPlayerState || !Definition)
	{
		NextRequirements.Reset();
		SetDisplayName(GetStatFallbackDisplayName(UpgradeType));
		SetCurrentLevel(1);
		SetMaxLevel(0);
		SetCurrentValue(0.0f);
		SetNextValue(0.0f);
		SetHasNextLevel(false);
		SetHasValidData(false);
		SetCanUpgrade(false);
		OnPreviewRefreshed.Broadcast();
		return;
	}

	const int32 NewCurrentLevel = CurrentPlayerState->GetWorkbenchUpgradeLevel(UpgradeType);
	const int32 NewMaxLevel = UpgradeData->GetMaxConfiguredLevel(UpgradeType);
	const FECWorkbenchUpgradeLevelData* CurrentLevelData =
		UpgradeData->FindLevelData(UpgradeType, NewCurrentLevel);
	const FECWorkbenchUpgradeLevelData* NextLevelData =
		UpgradeData->FindNextLevelData(UpgradeType, NewCurrentLevel);
	const FName ValueId = GetPrimaryValueId();

	float NewCurrentValue = 0.0f;
	float NewNextValue = 0.0f;
	const bool bFoundCurrentValue = FindUpgradeValue(CurrentLevelData, ValueId, NewCurrentValue);
	const bool bFoundNextValue = FindUpgradeValue(NextLevelData, ValueId, NewNextValue);
	NextRequirements = NextLevelData
		? NextLevelData->RequiredItems
		: TArray<FECInventoryItemRequirement>();

	SetDisplayName(Definition->DisplayName.IsEmpty()
		? GetStatFallbackDisplayName(UpgradeType)
		: Definition->DisplayName);
	SetCurrentLevel(NewCurrentLevel);
	SetMaxLevel(NewMaxLevel);
	SetCurrentValue(NewCurrentValue);
	SetNextValue(bFoundNextValue ? NewNextValue : NewCurrentValue);
	SetHasNextLevel(NextLevelData && bFoundNextValue);
	SetHasValidData(NewMaxLevel > 0 && bFoundCurrentValue);
	const bool bSupportsExecution =
		UpgradeType == EECWorkbenchUpgradeType::MaxHealth ||
		UpgradeType == EECWorkbenchUpgradeType::MaxStamina ||
		UpgradeType == EECWorkbenchUpgradeType::MaxInventoryWeight;
	SetCanUpgrade(
		bSupportsExecution &&
		CurrentPlayerState->CanExecuteWorkbenchUpgrade(UpgradeType));
	OnPreviewRefreshed.Broadcast();
}

bool UWorkbenchStatUpgradeViewModel::ExecuteUpgrade()
{
	AECPlayerState* CurrentPlayerState = PlayerState.Get();
	const bool bSupportsExecution =
		UpgradeType == EECWorkbenchUpgradeType::MaxHealth ||
		UpgradeType == EECWorkbenchUpgradeType::MaxStamina ||
		UpgradeType == EECWorkbenchUpgradeType::MaxInventoryWeight;
	if (!bSupportsExecution || !CurrentPlayerState)
	{
		return false;
	}

	return CurrentPlayerState->ExecuteWorkbenchUpgrade(UpgradeType);
}

void UWorkbenchStatUpgradeViewModel::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetUpgradeType);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(DisplayName);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentLevel);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(MaxLevel);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentValue);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(NextValue);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(bHasNextLevel);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(bHasValidData);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(bCanUpgrade);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentValueText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNextValueText);
}

void UWorkbenchStatUpgradeViewModel::BeginDestroy()
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

void UWorkbenchStatUpgradeViewModel::HandleInventoryUpdated()
{
	Refresh();
}

FText UWorkbenchStatUpgradeViewModel::GetCurrentValueText() const
{
	FNumberFormattingOptions Formatting;
	Formatting.MinimumFractionalDigits = 0;
	Formatting.MaximumFractionalDigits = 2;
	return FText::AsNumber(CurrentValue, &Formatting);
}

FText UWorkbenchStatUpgradeViewModel::GetNextValueText() const
{
	FNumberFormattingOptions Formatting;
	Formatting.MinimumFractionalDigits = 0;
	Formatting.MaximumFractionalDigits = 2;
	return FText::AsNumber(NextValue, &Formatting);
}

void UWorkbenchStatUpgradeViewModel::HandleUpgradeLevelChanged(
	EECWorkbenchUpgradeType ChangedType,
	int32 NewLevel)
{
	if (ChangedType == UpgradeType)
	{
		Refresh();
	}
}

void UWorkbenchStatUpgradeViewModel::SetDisplayName(const FText& NewDisplayName)
{
	UE_MVVM_SET_PROPERTY_VALUE(DisplayName, NewDisplayName);
}

void UWorkbenchStatUpgradeViewModel::SetCurrentLevel(int32 NewCurrentLevel)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentLevel, NewCurrentLevel);
}

void UWorkbenchStatUpgradeViewModel::SetMaxLevel(int32 NewMaxLevel)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxLevel, NewMaxLevel);
}

void UWorkbenchStatUpgradeViewModel::SetCurrentValue(float NewCurrentValue)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(CurrentValue, NewCurrentValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentValueText);
	}
}

void UWorkbenchStatUpgradeViewModel::SetNextValue(float NewNextValue)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(NextValue, NewNextValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNextValueText);
	}
}

void UWorkbenchStatUpgradeViewModel::SetHasNextLevel(bool bNewHasNextLevel)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasNextLevel, bNewHasNextLevel);
}

void UWorkbenchStatUpgradeViewModel::SetHasValidData(bool bNewHasValidData)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasValidData, bNewHasValidData);
}

void UWorkbenchStatUpgradeViewModel::SetCanUpgrade(bool bNewCanUpgrade)
{
	UE_MVVM_SET_PROPERTY_VALUE(bCanUpgrade, bNewCanUpgrade);
}

FName UWorkbenchStatUpgradeViewModel::GetPrimaryValueId() const
{
	switch (UpgradeType)
	{
	case EECWorkbenchUpgradeType::MaxHealth:
		return ECWorkbenchStatValueIds::MaxHealth;
	case EECWorkbenchUpgradeType::MaxStamina:
		return ECWorkbenchStatValueIds::MaxStamina;
	case EECWorkbenchUpgradeType::MaxInventoryWeight:
		return ECWorkbenchStatValueIds::MaxInventoryWeight;
	default:
		return NAME_None;
	}
}
