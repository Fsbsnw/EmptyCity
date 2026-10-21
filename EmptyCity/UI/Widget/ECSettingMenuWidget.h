#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECSettingMenuWidget.generated.h"

class UButton;
class UOverlay;
class UWidgetSwitcher;

/**
 * 설정 메뉴 안에서 디스플레이/마우스·키보드 페이지 전환을 담당합니다.
 * WBP_SettingMenu의 동일한 이름을 가진 위젯과 자동으로 바인딩됩니다.
 */
UCLASS(Abstract, Blueprintable)
class EMPTYCITY_API UECSettingMenuWidget : public UECUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "UI|Settings")
	void ShowDisplaySettings();

	UFUNCTION(BlueprintCallable, Category = "UI|Settings")
	void ShowMouseKeySettings();

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_DisplaySetting;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_MouseKeySetting;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Apply;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Reset;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_DisplaySetting;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_MouseKeySetting;
};
