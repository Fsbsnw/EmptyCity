#include "UI/Widget/Workbench/ECWorkbenchStatUpgradeWidget.h"

#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "UI/ViewModel/WorkbenchStatUpgradeViewModel.h"
#include "UI/Widget/Workbench/ECWorkbenchUpgradeRequirementWidget.h"

void UECWorkbenchStatUpgradeWidget::InjectViewModel(UMVVMViewModelBase* TargetViewModel)
{
	UWorkbenchStatUpgradeViewModel* InjectedStatViewModel =
		Cast<UWorkbenchStatUpgradeViewModel>(TargetViewModel);
	if (!InjectedStatViewModel)
	{
		Super::InjectViewModel(TargetViewModel);
		return;
	}

	// UIManager는 WBP_Workbench에 ViewModel을 한 번 생성합니다. 같은 인스턴스를
	// HP/Stamina/Weight 카드가 함께 사용하면 마지막 SetUpgradeType이 모든 카드를
	// 덮어쓰므로, 외부 인스턴스의 클래스만 이어받아 카드별 인스턴스를 유지합니다.
	UWorkbenchStatUpgradeViewModel* CardViewModel = InjectedStatViewModel;
	if (InjectedStatViewModel->GetOuter() != this)
	{
		if (StatUpgradeViewModel
			&& StatUpgradeViewModel->GetOuter() == this
			&& StatUpgradeViewModel->GetClass() == InjectedStatViewModel->GetClass())
		{
			CardViewModel = StatUpgradeViewModel;
		}
		else
		{
			CardViewModel = NewObject<UWorkbenchStatUpgradeViewModel>(
				this,
				InjectedStatViewModel->GetClass());
		}
	}

	if (StatUpgradeViewModel && StatUpgradeViewModel != CardViewModel)
	{
		StatUpgradeViewModel->OnPreviewRefreshed.RemoveAll(this);
	}

	StatUpgradeViewModel = CardViewModel;
	Super::InjectViewModel(StatUpgradeViewModel);

	// UIManager의 OnViewModelSet은 위젯이 패널에 붙기 전에 호출될 수 있습니다.
	// WidgetTree/BindWidget은 이미 준비되어 있으므로 즉시 카드 타입으로 초기화합니다.
	InitializeViewModel();
}

void UECWorkbenchStatUpgradeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Upgrade)
	{
		Button_Upgrade->OnClicked.RemoveDynamic(this, &ThisClass::HandleUpgradeClicked);
		Button_Upgrade->OnClicked.AddDynamic(this, &ThisClass::HandleUpgradeClicked);
	}

	InitializeViewModel();
}

void UECWorkbenchStatUpgradeWidget::NativeDestruct()
{
	if (Button_Upgrade)
	{
		Button_Upgrade->OnClicked.RemoveDynamic(this, &ThisClass::HandleUpgradeClicked);
	}
	if (StatUpgradeViewModel)
	{
		StatUpgradeViewModel->OnPreviewRefreshed.RemoveAll(this);
	}

	Super::NativeDestruct();
}
void UECWorkbenchStatUpgradeWidget::SetUpgradeType(EECWorkbenchUpgradeType NewUpgradeType)
{
	UpgradeType = NewUpgradeType;

	if (StatUpgradeViewModel)
	{
		StatUpgradeViewModel->SetUpgradeType(UpgradeType);
	}
}

void UECWorkbenchStatUpgradeWidget::RefreshStatPreview()
{
	if (!StatUpgradeViewModel)
	{
		InitializeViewModel();
		return;
	}

	StatUpgradeViewModel->Refresh();
}

void UECWorkbenchStatUpgradeWidget::HandleUpgradeClicked()
{
	if (StatUpgradeViewModel && StatUpgradeViewModel->ExecuteUpgrade())
	{
		// 단계 변경/인벤토리 갱신 델리게이트에서도 새로고침되지만,
		// 버튼 클릭 직후의 상태를 확실히 맞춥니다.
		StatUpgradeViewModel->Refresh();
	}
}

void UECWorkbenchStatUpgradeWidget::RefreshCardState()
{
	if (Button_Upgrade)
	{
		const bool bSupportsExecution =
			UpgradeType == EECWorkbenchUpgradeType::MaxHealth ||
			UpgradeType == EECWorkbenchUpgradeType::MaxStamina ||
			UpgradeType == EECWorkbenchUpgradeType::MaxInventoryWeight;
		Button_Upgrade->SetIsEnabled(
			bSupportsExecution &&
			StatUpgradeViewModel &&
			StatUpgradeViewModel->CanUpgrade());
	}

	RebuildRequirementWidgets();
}

void UECWorkbenchStatUpgradeWidget::RebuildRequirementWidgets()
{
	if (!Panel_RequiredItems)
	{
		return;
	}

	Panel_RequiredItems->ClearChildren();

	// 부모 WBP에 이미 배치된 자식 UserWidget 템플릿은, 자식 WBP의 Class Defaults를
	// 나중에 변경해도 예전 nullptr 값을 인스턴스 override로 유지할 수 있습니다.
	// 에셋 경로를 하드코딩하지 않고 실제 자식 WBP CDO에 설정된 클래스를 사용합니다.
	TSubclassOf<UECWorkbenchUpgradeRequirementWidget> EffectiveRequirementWidgetClass =
		RequirementWidgetClass;
	if (!EffectiveRequirementWidgetClass)
	{
		const UECWorkbenchStatUpgradeWidget* ClassDefaults =
			GetClass()->GetDefaultObject<UECWorkbenchStatUpgradeWidget>();
		if (ClassDefaults && ClassDefaults != this)
		{
			EffectiveRequirementWidgetClass = ClassDefaults->RequirementWidgetClass;
		}
	}

	if (!StatUpgradeViewModel || !EffectiveRequirementWidgetClass)
	{
		return;
	}

	for (const FECInventoryItemRequirement& Requirement : StatUpgradeViewModel->GetNextRequirements())
	{
		UECWorkbenchUpgradeRequirementWidget* RequirementWidget =
			CreateWidget<UECWorkbenchUpgradeRequirementWidget>(this, EffectiveRequirementWidgetClass);
		if (!RequirementWidget)
		{
			continue;
		}

		RequirementWidget->SetRequirement(Requirement);
		Panel_RequiredItems->AddChild(RequirementWidget);
	}
}

void UECWorkbenchStatUpgradeWidget::InitializeViewModel()
{
	if (!StatUpgradeViewModel)
	{
		StatUpgradeViewModel = NewObject<UWorkbenchStatUpgradeViewModel>(this);
	}

	StatUpgradeViewModel->OnPreviewRefreshed.RemoveAll(this);
	StatUpgradeViewModel->OnPreviewRefreshed.AddUObject(this, &ThisClass::RefreshCardState);
	// 외부 주입 override를 다시 타지 않고 이 카드의 최종 인스턴스를 MVVM에 등록합니다.
	UECUserWidget::InjectViewModel(StatUpgradeViewModel);
	StatUpgradeViewModel->Initialize(GetOwningPlayer(), UpgradeType);
	StatUpgradeViewModel->BroadcastInitialValues();
	OnViewModelSet();
	RefreshCardState();
}
