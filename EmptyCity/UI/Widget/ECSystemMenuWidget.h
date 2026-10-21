#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECSystemMenuWidget.generated.h"

class UButton;

/**
 * 블루프린트 시스템 메뉴의 동작만 담당하는 베이스 클래스입니다.
 * 같은 이름의 위젯을 WBP 디자이너에서 생성해야 합니다.
 */
UCLASS(Abstract, Blueprintable)
class EMPTYCITY_API UECSystemMenuWidget : public UECUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void HandleSaveClicked();

	UFUNCTION()
	void HandleLoadClicked();

	UFUNCTION()
	void HandleSettingsClicked();

	UFUNCTION()
	void HandleMainMenuClicked();

	UFUNCTION()
	void HandleQuitClicked();

	/** 세이브 파일 없이 8일차 오전으로 디버그 시작합니다. */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void HandleDebugDay8Clicked();

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> SaveButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> LoadButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> MainMenuButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

	/** WBP에 같은 이름의 버튼을 추가하면 클릭 이벤트가 자동 연결됩니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> DebugDay8Button;
};
