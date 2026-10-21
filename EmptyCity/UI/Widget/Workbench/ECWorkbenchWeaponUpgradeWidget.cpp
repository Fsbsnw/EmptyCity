#include "UI/Widget/Workbench/ECWorkbenchWeaponUpgradeWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Engine/Texture2D.h"
#include "UI/ViewModel/WorkbenchWeaponUpgradeViewModel.h"
#include "UI/Widget/Workbench/ECWorkbenchModelPreviewWidget.h"
#include "UI/Widget/Workbench/ECWorkbenchUpgradeRequirementWidget.h"

void UECWorkbenchWeaponUpgradeWidget::InjectViewModel(UMVVMViewModelBase* TargetViewModel)
{
	UWorkbenchWeaponUpgradeViewModel* InjectedViewModel =
		Cast<UWorkbenchWeaponUpgradeViewModel>(TargetViewModel);
	if (!InjectedViewModel)
	{
		Super::InjectViewModel(TargetViewModel);
		return;
	}

	if (WeaponUpgradeViewModel && WeaponUpgradeViewModel != InjectedViewModel)
	{
		WeaponUpgradeViewModel->OnPreviewRefreshed.RemoveAll(this);
	}

	WeaponUpgradeViewModel = InjectedViewModel;
	Super::InjectViewModel(WeaponUpgradeViewModel);
	InitializeViewModel();
}

void UECWorkbenchWeaponUpgradeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (WidgetTree && WidgetTree->RootWidget)
	{
		WidgetTree->RootWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (Button_Upgrade)
	{
		Button_Upgrade->OnClicked.RemoveDynamic(this, &ThisClass::HandleUpgradeClicked);
		Button_Upgrade->OnClicked.AddDynamic(this, &ThisClass::HandleUpgradeClicked);
	}

	InitializeViewModel();
}

void UECWorkbenchWeaponUpgradeWidget::NativeDestruct()
{
	if (Button_Upgrade)
	{
		Button_Upgrade->OnClicked.RemoveDynamic(this, &ThisClass::HandleUpgradeClicked);
	}
	if (WeaponUpgradeViewModel)
	{
		WeaponUpgradeViewModel->OnPreviewRefreshed.RemoveAll(this);
	}

	Super::NativeDestruct();
}

FReply UECWorkbenchWeaponUpgradeWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	return FReply::Unhandled();
}

void UECWorkbenchWeaponUpgradeWidget::RefreshWeaponPreview()
{
	if (WeaponUpgradeViewModel)
	{
		WeaponUpgradeViewModel->Refresh();
	}
	else
	{
		InitializeViewModel();
	}
}

void UECWorkbenchWeaponUpgradeWidget::HandleUpgradeClicked()
{
	if (WeaponUpgradeViewModel && WeaponUpgradeViewModel->ExecuteUpgrade())
	{
		// 강화 트랜잭션이 끝난 뒤의 새 레벨을 다시 읽어 같은 프레임에 이미지를 교체합니다.
		// PlayerState/Inventory 델리게이트 순서에 의존하지 않도록 명시적으로 갱신합니다.
		WeaponUpgradeViewModel->Refresh();
		ApplyLevelImage(WeaponUpgradeViewModel->GetCurrentLevel());
	}
}

void UECWorkbenchWeaponUpgradeWidget::RefreshWidgetState()
{
	if (Button_Upgrade)
	{
		Button_Upgrade->SetIsEnabled(
			WeaponUpgradeViewModel && WeaponUpgradeViewModel->CanUpgrade());
	}

	RebuildRequirementWidgets();
	const int32 CurrentLevel = WeaponUpgradeViewModel
		? WeaponUpgradeViewModel->GetCurrentLevel()
		: 1;
	ApplyLevelImage(CurrentLevel);
	if (ModelPreviewWidget)
	{
		ModelPreviewWidget->SetPreviewLevel(CurrentLevel);
	}
	OnWeaponPreviewChanged(CurrentLevel);
}

void UECWorkbenchWeaponUpgradeWidget::ApplyLevelImage(int32 CurrentLevel)
{
	if (!Image_Background)
	{
		return;
	}

	UTexture2D* TargetImage = nullptr;
	switch (FMath::Clamp(CurrentLevel, 1, 3))
	{
	case 1:
		TargetImage = Level1Image;
		break;
	case 2:
		TargetImage = Level2Image;
		break;
	case 3:
		TargetImage = Level3Image;
		break;
	default:
		break;
	}

	if (TargetImage)
	{
		// 위젯의 블루프린트 레이아웃 크기는 유지하고 브러시 리소스만 교체합니다.
		FSlateBrush UpdatedBrush = Image_Background->GetBrush();
		UpdatedBrush.SetResourceObject(TargetImage);
		Image_Background->SetBrush(UpdatedBrush);
		Image_Background->InvalidateLayoutAndVolatility();
	}
}

void UECWorkbenchWeaponUpgradeWidget::RebuildRequirementWidgets()
{
	if (!Panel_RequiredItems)
	{
		return;
	}

	Panel_RequiredItems->ClearChildren();
	TSubclassOf<UECWorkbenchUpgradeRequirementWidget> EffectiveClass = RequirementWidgetClass;
	if (!EffectiveClass)
	{
		const UECWorkbenchWeaponUpgradeWidget* ClassDefaults =
			GetClass()->GetDefaultObject<UECWorkbenchWeaponUpgradeWidget>();
		if (ClassDefaults && ClassDefaults != this)
		{
			EffectiveClass = ClassDefaults->RequirementWidgetClass;
		}
	}

	if (!WeaponUpgradeViewModel || !EffectiveClass)
	{
		return;
	}

	for (const FECInventoryItemRequirement& Requirement
		: WeaponUpgradeViewModel->GetNextRequirements())
	{
		UECWorkbenchUpgradeRequirementWidget* RequirementWidget =
			CreateWidget<UECWorkbenchUpgradeRequirementWidget>(this, EffectiveClass);
		if (RequirementWidget)
		{
			RequirementWidget->SetRequirement(Requirement);
			Panel_RequiredItems->AddChild(RequirementWidget);
		}
	}
}

void UECWorkbenchWeaponUpgradeWidget::InitializeViewModel()
{
	if (!WeaponUpgradeViewModel)
	{
		WeaponUpgradeViewModel = NewObject<UWorkbenchWeaponUpgradeViewModel>(this);
	}

	WeaponUpgradeViewModel->OnPreviewRefreshed.RemoveAll(this);
	WeaponUpgradeViewModel->OnPreviewRefreshed.AddUObject(
		this,
		&ThisClass::RefreshWidgetState);
	UECUserWidget::InjectViewModel(WeaponUpgradeViewModel);
	WeaponUpgradeViewModel->Initialize(GetOwningPlayer());
	WeaponUpgradeViewModel->BroadcastInitialValues();
	OnViewModelSet();
	RefreshWidgetState();
}
