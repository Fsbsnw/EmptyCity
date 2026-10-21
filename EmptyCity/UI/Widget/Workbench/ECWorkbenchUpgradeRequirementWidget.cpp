#include "UI/Widget/Workbench/ECWorkbenchUpgradeRequirementWidget.h"

#include "UI/ViewModel/WorkbenchUpgradeRequirementViewModel.h"

void UECWorkbenchUpgradeRequirementWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!RequirementViewModel)
	{
		RequirementViewModel = NewObject<UWorkbenchUpgradeRequirementViewModel>(this);
	}

	InjectViewModel(RequirementViewModel);
	RequirementViewModel->Initialize(GetOwningPlayer(), Requirement);
	RequirementViewModel->BroadcastInitialValues();
	OnViewModelSet();
}

void UECWorkbenchUpgradeRequirementWidget::SetRequirement(
	const FECInventoryItemRequirement& NewRequirement)
{
	Requirement = NewRequirement;

	if (RequirementViewModel)
	{
		RequirementViewModel->SetRequirement(Requirement);
	}
}

