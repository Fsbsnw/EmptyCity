#include "UI/Widget/ECSettingMenuWidget.h"

#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/WidgetSwitcher.h"

void UECSettingMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_DisplaySetting->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowDisplaySettings);
	Button_MouseKeySetting->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowMouseKeySettings);
	
	Button_DisplaySetting->SetIsEnabled(false);
	Button_MouseKeySetting->SetIsEnabled(true);

	ShowDisplaySettings();
}

void UECSettingMenuWidget::ShowDisplaySettings()
{
	WidgetSwitcher->SetActiveWidget(Overlay_DisplaySetting);
	Button_MouseKeySetting->SetIsEnabled(true);
	Button_DisplaySetting->SetIsEnabled(false);
}

void UECSettingMenuWidget::ShowMouseKeySettings()
{
	WidgetSwitcher->SetActiveWidget(Overlay_MouseKeySetting);
	Button_MouseKeySetting->SetIsEnabled(false);
	Button_DisplaySetting->SetIsEnabled(true);
}
