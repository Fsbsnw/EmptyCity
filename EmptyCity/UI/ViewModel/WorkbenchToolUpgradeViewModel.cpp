#include "UI/ViewModel/WorkbenchToolUpgradeViewModel.h"

#include "Character/Player/Controller/ECPlayerController.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Data/Workbench/ECWorkbenchUpgradeDataAsset.h"
#include "Inventory/ECInventoryManagerComponent.h"

namespace
{
	const FName ToolWeightValueId(TEXT("Weight"));
	const FName ToolInteractionDurationValueId(TEXT("InteractionDuration"));

	bool IsToolUpgradeType(EECWorkbenchUpgradeType UpgradeType)
	{
		return UpgradeType == EECWorkbenchUpgradeType::Axe
			|| UpgradeType == EECWorkbenchUpgradeType::Pickaxe;
	}

	bool FindValue(
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

	FText GetToolFallbackDisplayName(EECWorkbenchUpgradeType UpgradeType)
	{
		return UpgradeType == EECWorkbenchUpgradeType::Pickaxe
			? NSLOCTEXT("Workbench", "PickaxeDisplayName", "곡괭이")
			: NSLOCTEXT("Workbench", "AxeDisplayName", "도끼");
	}
}

void UWorkbenchToolUpgradeViewModel::Initialize(
	AActor* ContextActor,
	EECWorkbenchUpgradeType InSelectedTool)
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

	SetSelectedTool(InSelectedTool);
}

void UWorkbenchToolUpgradeViewModel::SetSelectedTool(EECWorkbenchUpgradeType InSelectedTool)
{
	const EECWorkbenchUpgradeType SanitizedTool = IsToolUpgradeType(InSelectedTool)
		? InSelectedTool
		: EECWorkbenchUpgradeType::Axe;
	if (SelectedTool != SanitizedTool)
	{
		SelectedTool = SanitizedTool;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetSelectedTool);
	}

	Refresh();
}

void UWorkbenchToolUpgradeViewModel::Refresh()
{
	const AECPlayerState* CurrentPlayerState = PlayerState.Get();
	const UECWorkbenchUpgradeDataAsset* UpgradeData =
		CurrentPlayerState ? CurrentPlayerState->GetWorkbenchUpgradeData() : nullptr;
	const FECWorkbenchUpgradeDefinition* Definition =
		UpgradeData ? UpgradeData->FindUpgradeDefinition(SelectedTool) : nullptr;

	if (!CurrentPlayerState || !Definition)
	{
		NextRequirements.Reset();
		SetDisplayName(GetToolFallbackDisplayName(SelectedTool));
		SetCurrentLevel(1);
		SetMaxLevel(0);
		SetCurrentWeight(0.0f);
		SetNextWeight(0.0f);
		SetCurrentInteractionDuration(0.0f);
		SetNextInteractionDuration(0.0f);
		SetHasNextLevel(false);
		SetHasValidData(false);
		SetCanUpgrade(false);
		OnPreviewRefreshed.Broadcast();
		return;
	}

	const int32 NewCurrentLevel = CurrentPlayerState->GetWorkbenchUpgradeLevel(SelectedTool);
	const int32 NewMaxLevel = UpgradeData->GetMaxConfiguredLevel(SelectedTool);
	const FECWorkbenchUpgradeLevelData* CurrentLevelData =
		UpgradeData->FindLevelData(SelectedTool, NewCurrentLevel);
	const FECWorkbenchUpgradeLevelData* NextLevelData =
		UpgradeData->FindNextLevelData(SelectedTool, NewCurrentLevel);

	float NewCurrentWeight = 0.0f;
	float NewNextWeight = 0.0f;
	float NewCurrentInteractionDuration = 0.0f;
	float NewNextInteractionDuration = 0.0f;
	const bool bFoundCurrentWeight = FindValue(CurrentLevelData, ToolWeightValueId, NewCurrentWeight);
	const bool bFoundNextWeight = FindValue(NextLevelData, ToolWeightValueId, NewNextWeight);
	const bool bFoundCurrentInteractionDuration =
		FindValue(CurrentLevelData, ToolInteractionDurationValueId, NewCurrentInteractionDuration)
		&& NewCurrentInteractionDuration > UE_KINDA_SMALL_NUMBER;
	const bool bFoundNextInteractionDuration =
		FindValue(NextLevelData, ToolInteractionDurationValueId, NewNextInteractionDuration)
		&& NewNextInteractionDuration > UE_KINDA_SMALL_NUMBER;
	NextRequirements = NextLevelData
		? NextLevelData->RequiredItems
		: TArray<FECInventoryItemRequirement>();

	SetDisplayName(Definition->DisplayName.IsEmpty()
		? GetToolFallbackDisplayName(SelectedTool)
		: Definition->DisplayName);
	SetCurrentLevel(NewCurrentLevel);
	SetMaxLevel(NewMaxLevel);
	SetCurrentWeight(NewCurrentWeight);
	SetNextWeight(bFoundNextWeight ? NewNextWeight : NewCurrentWeight);
	SetCurrentInteractionDuration(
		bFoundCurrentInteractionDuration ? NewCurrentInteractionDuration : 0.0f);
	SetNextInteractionDuration(
		bFoundNextInteractionDuration
			? NewNextInteractionDuration
			: NewCurrentInteractionDuration);
	SetHasNextLevel(NextLevelData && bFoundNextWeight && bFoundNextInteractionDuration);
	SetHasValidData(NewMaxLevel > 0 && bFoundCurrentWeight && bFoundCurrentInteractionDuration);
	SetCanUpgrade(
		bFoundCurrentWeight
		&& bFoundCurrentInteractionDuration
		&& NextLevelData
		&& bFoundNextWeight
		&& bFoundNextInteractionDuration
		&& CurrentPlayerState->CanExecuteWorkbenchUpgrade(SelectedTool));
	OnPreviewRefreshed.Broadcast();
}

bool UWorkbenchToolUpgradeViewModel::ExecuteUpgrade()
{
	AECPlayerState* CurrentPlayerState = PlayerState.Get();
	return CurrentPlayerState
		&& IsToolUpgradeType(SelectedTool)
		&& CurrentPlayerState->ExecuteWorkbenchUpgrade(SelectedTool);
}

void UWorkbenchToolUpgradeViewModel::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetSelectedTool);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(DisplayName);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentLevel);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(MaxLevel);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentWeight);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(NextWeight);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentInteractionDuration);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(NextInteractionDuration);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(bHasNextLevel);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(bHasValidData);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(bCanUpgrade);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentWeightText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNextWeightText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentInteractionDurationText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNextInteractionDurationText);
}

void UWorkbenchToolUpgradeViewModel::BeginDestroy()
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

void UWorkbenchToolUpgradeViewModel::HandleUpgradeLevelChanged(
	EECWorkbenchUpgradeType ChangedType,
	int32 NewLevel)
{
	if (ChangedType == SelectedTool)
	{
		Refresh();
	}
}

void UWorkbenchToolUpgradeViewModel::HandleInventoryUpdated()
{
	Refresh();
}

FText UWorkbenchToolUpgradeViewModel::GetCurrentWeightText() const
{
	FNumberFormattingOptions Formatting;
	Formatting.MinimumFractionalDigits = 0;
	Formatting.MaximumFractionalDigits = 2;
	return FText::AsNumber(CurrentWeight, &Formatting);
}

FText UWorkbenchToolUpgradeViewModel::GetNextWeightText() const
{
	FNumberFormattingOptions Formatting;
	Formatting.MinimumFractionalDigits = 0;
	Formatting.MaximumFractionalDigits = 2;
	return FText::AsNumber(NextWeight, &Formatting);
}

FText UWorkbenchToolUpgradeViewModel::GetCurrentInteractionDurationText() const
{
	FNumberFormattingOptions Formatting;
	Formatting.MinimumFractionalDigits = 0;
	Formatting.MaximumFractionalDigits = 2;
	return FText::AsNumber(CurrentInteractionDuration, &Formatting);
}

FText UWorkbenchToolUpgradeViewModel::GetNextInteractionDurationText() const
{
	FNumberFormattingOptions Formatting;
	Formatting.MinimumFractionalDigits = 0;
	Formatting.MaximumFractionalDigits = 2;
	return FText::AsNumber(NextInteractionDuration, &Formatting);
}

void UWorkbenchToolUpgradeViewModel::SetDisplayName(const FText& NewDisplayName)
{
	UE_MVVM_SET_PROPERTY_VALUE(DisplayName, NewDisplayName);
}

void UWorkbenchToolUpgradeViewModel::SetCurrentLevel(int32 NewCurrentLevel)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentLevel, NewCurrentLevel);
}

void UWorkbenchToolUpgradeViewModel::SetMaxLevel(int32 NewMaxLevel)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxLevel, NewMaxLevel);
}

void UWorkbenchToolUpgradeViewModel::SetCurrentWeight(float NewCurrentWeight)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(CurrentWeight, NewCurrentWeight))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentWeightText);
	}
}

void UWorkbenchToolUpgradeViewModel::SetNextWeight(float NewNextWeight)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(NextWeight, NewNextWeight))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNextWeightText);
	}
}

void UWorkbenchToolUpgradeViewModel::SetCurrentInteractionDuration(float NewDuration)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(CurrentInteractionDuration, NewDuration))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentInteractionDurationText);
	}
}

void UWorkbenchToolUpgradeViewModel::SetNextInteractionDuration(float NewDuration)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(NextInteractionDuration, NewDuration))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNextInteractionDurationText);
	}
}

void UWorkbenchToolUpgradeViewModel::SetHasNextLevel(bool bNewHasNextLevel)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasNextLevel, bNewHasNextLevel);
}

void UWorkbenchToolUpgradeViewModel::SetHasValidData(bool bNewHasValidData)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasValidData, bNewHasValidData);
}

void UWorkbenchToolUpgradeViewModel::SetCanUpgrade(bool bNewCanUpgrade)
{
	UE_MVVM_SET_PROPERTY_VALUE(bCanUpgrade, bNewCanUpgrade);
}
