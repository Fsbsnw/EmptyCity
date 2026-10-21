#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ECMainMenuWidget.generated.h"

class UButton;
class UImage;

/** 메인 메뉴의 각 버튼이 호버될 때 해당 버튼의 백라이트 이미지를 표시합니다. */
UCLASS()
class EMPTYCITY_API UECMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleStartGameHovered();

	UFUNCTION()
	void HandleStartGameUnhovered();

	UFUNCTION()
	void HandleLoadGameHovered();

	UFUNCTION()
	void HandleLoadGameUnhovered();

	UFUNCTION()
	void HandleSettingHovered();

	UFUNCTION()
	void HandleSettingUnhovered();

	UFUNCTION()
	void HandleQuitGameHovered();

	UFUNCTION()
	void HandleQuitGameUnhovered();

	/** 세이브 파일 없이 8일차 오전으로 디버그 시작합니다. */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void HandleDebugDay8Clicked();

	static void SetBackLightVisible(UImage* BackLight, bool bVisible);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_StartGame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_LoadGame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Setting;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_QuitGame;

	/** WBP에 같은 이름의 버튼을 추가하면 클릭 이벤트가 자동 연결됩니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> Button_DebugDay8;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_StartGameBackLight;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_LoadGameBackLight;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_SettingBackLight;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_QuitGameBackLight;
};
