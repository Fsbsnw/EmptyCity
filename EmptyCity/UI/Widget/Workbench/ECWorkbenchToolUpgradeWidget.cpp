#include "UI/Widget/Workbench/ECWorkbenchToolUpgradeWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Engine/Texture2D.h"
#include "UI/ViewModel/WorkbenchToolUpgradeViewModel.h"
#include "UI/Widget/Workbench/ECWorkbenchModelPreviewWidget.h"
#include "UI/Widget/Workbench/ECWorkbenchUpgradeRequirementWidget.h"

void UECWorkbenchToolUpgradeWidget::InjectViewModel(UMVVMViewModelBase* TargetViewModel)
{
	UWorkbenchToolUpgradeViewModel* InjectedViewModel =
		Cast<UWorkbenchToolUpgradeViewModel>(TargetViewModel);
	if (!InjectedViewModel)
	{
		Super::InjectViewModel(TargetViewModel);
		return;
	}

	if (ToolUpgradeViewModel && ToolUpgradeViewModel != InjectedViewModel)
	{
		ToolUpgradeViewModel->OnPreviewRefreshed.RemoveAll(this);
	}

	ToolUpgradeViewModel = InjectedViewModel;
	Super::InjectViewModel(ToolUpgradeViewModel);
	InitializeViewModel();
}

void UECWorkbenchToolUpgradeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (WidgetTree && WidgetTree->RootWidget)
	{
		// WBP의 전체 화면 Root Overlay 자체는 빈 영역 입력을 잡지 않습니다.
		// 자식 Button_Upgrade 같은 실제 조작 위젯은 계속 HitTest에 참여합니다.
		WidgetTree->RootWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (Button_Upgrade)
	{
		Button_Upgrade->OnClicked.RemoveDynamic(this, &ThisClass::HandleUpgradeClicked);
		Button_Upgrade->OnClicked.AddDynamic(this, &ThisClass::HandleUpgradeClicked);
	}

	InitializeViewModel();
}

void UECWorkbenchToolUpgradeWidget::NativeDestruct()
{
	if (Button_Upgrade)
	{
		Button_Upgrade->OnClicked.RemoveDynamic(this, &ThisClass::HandleUpgradeClicked);
	}
	if (ToolUpgradeViewModel)
	{
		ToolUpgradeViewModel->OnPreviewRefreshed.RemoveAll(this);
	}

	Super::NativeDestruct();
}

FReply UECWorkbenchToolUpgradeWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	// ECUserWidget의 기본 구현은 드래그를 사용하지 않아도 입력을 소비합니다.
	// 전체 화면 크기의 도구 상세 배경은 상위 Workbench 탭 입력을 통과시킵니다.
	return FReply::Unhandled();
}

void UECWorkbenchToolUpgradeWidget::SelectTool(EECWorkbenchUpgradeType NewTool)
{
	SelectedTool = NewTool == EECWorkbenchUpgradeType::Pickaxe
		? EECWorkbenchUpgradeType::Pickaxe
		: EECWorkbenchUpgradeType::Axe;

	if (ToolUpgradeViewModel)
	{
		ToolUpgradeViewModel->SetSelectedTool(SelectedTool);
	}
	else
	{
		InitializeViewModel();
	}
}

void UECWorkbenchToolUpgradeWidget::RefreshToolPreview()
{
	if (ToolUpgradeViewModel)
	{
		ToolUpgradeViewModel->Refresh();
	}
	else
	{
		InitializeViewModel();
	}
}

void UECWorkbenchToolUpgradeWidget::HandleUpgradeClicked()
{
	if (ToolUpgradeViewModel && ToolUpgradeViewModel->ExecuteUpgrade())
	{
		// 강화 완료 직후 새 단계의 이미지를 같은 프레임에 반영합니다.
		ToolUpgradeViewModel->Refresh();
		ApplyLevelImage(ToolUpgradeViewModel->GetCurrentLevel());
	}
}

void UECWorkbenchToolUpgradeWidget::RefreshWidgetState()
{
	if (Button_Upgrade)
	{
		Button_Upgrade->SetIsEnabled(ToolUpgradeViewModel && ToolUpgradeViewModel->CanUpgrade());
	}

	RebuildRequirementWidgets();
	const int32 CurrentLevel = ToolUpgradeViewModel
		? ToolUpgradeViewModel->GetCurrentLevel()
		: 1;
	ApplyLevelImage(CurrentLevel);
	if (ModelPreviewWidget)
	{
		ModelPreviewWidget->SetPreviewLevel(CurrentLevel);
	}
	OnToolPreviewChanged(SelectedTool, CurrentLevel);
}

void UECWorkbenchToolUpgradeWidget::ApplyLevelImage(int32 CurrentLevel)
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
		// 블루프린트에서 정한 이미지 크기는 유지하고 텍스처만 교체합니다.
		FSlateBrush UpdatedBrush = Image_Background->GetBrush();
		UpdatedBrush.SetResourceObject(TargetImage);
		Image_Background->SetBrush(UpdatedBrush);
		Image_Background->InvalidateLayoutAndVolatility();
	}
}

void UECWorkbenchToolUpgradeWidget::RebuildRequirementWidgets()
{
	if (!Panel_RequiredItems)
	{
		return;
	}

	Panel_RequiredItems->ClearChildren();
	TSubclassOf<UECWorkbenchUpgradeRequirementWidget> EffectiveClass = RequirementWidgetClass;
	if (!EffectiveClass)
	{
		const UECWorkbenchToolUpgradeWidget* ClassDefaults =
			GetClass()->GetDefaultObject<UECWorkbenchToolUpgradeWidget>();
		if (ClassDefaults && ClassDefaults != this)
		{
			EffectiveClass = ClassDefaults->RequirementWidgetClass;
		}
	}

	if (!ToolUpgradeViewModel || !EffectiveClass)
	{
		return;
	}

	for (const FECInventoryItemRequirement& Requirement : ToolUpgradeViewModel->GetNextRequirements())
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

void UECWorkbenchToolUpgradeWidget::InitializeViewModel()
{
	if (!ToolUpgradeViewModel)
	{
		ToolUpgradeViewModel = NewObject<UWorkbenchToolUpgradeViewModel>(this);
	}

	ToolUpgradeViewModel->OnPreviewRefreshed.RemoveAll(this);
	ToolUpgradeViewModel->OnPreviewRefreshed.AddUObject(this, &ThisClass::RefreshWidgetState);
	UECUserWidget::InjectViewModel(ToolUpgradeViewModel);
	ToolUpgradeViewModel->Initialize(GetOwningPlayer(), SelectedTool);
	ToolUpgradeViewModel->BroadcastInitialValues();
	OnViewModelSet();
	RefreshWidgetState();
}
