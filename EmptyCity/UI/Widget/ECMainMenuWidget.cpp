#include "UI/Widget/ECMainMenuWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Core/GameInstance/ECGameInstance.h"

void UECMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Button_StartGame->OnHovered.AddUniqueDynamic(this, &ThisClass::HandleStartGameHovered);
	Button_StartGame->OnUnhovered.AddUniqueDynamic(this, &ThisClass::HandleStartGameUnhovered);
	Button_LoadGame->OnHovered.AddUniqueDynamic(this, &ThisClass::HandleLoadGameHovered);
	Button_LoadGame->OnUnhovered.AddUniqueDynamic(this, &ThisClass::HandleLoadGameUnhovered);
	Button_Setting->OnHovered.AddUniqueDynamic(this, &ThisClass::HandleSettingHovered);
	Button_Setting->OnUnhovered.AddUniqueDynamic(this, &ThisClass::HandleSettingUnhovered);
	Button_QuitGame->OnHovered.AddUniqueDynamic(this, &ThisClass::HandleQuitGameHovered);
	Button_QuitGame->OnUnhovered.AddUniqueDynamic(this, &ThisClass::HandleQuitGameUnhovered);
	if (Button_DebugDay8)
	{
		Button_DebugDay8->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleDebugDay8Clicked);
	}

	SetBackLightVisible(Image_StartGameBackLight, false);
	SetBackLightVisible(Image_LoadGameBackLight, false);
	SetBackLightVisible(Image_SettingBackLight, false);
	SetBackLightVisible(Image_QuitGameBackLight, false);
}

void UECMainMenuWidget::NativeDestruct()
{
	Button_StartGame->OnHovered.RemoveDynamic(this, &ThisClass::HandleStartGameHovered);
	Button_StartGame->OnUnhovered.RemoveDynamic(this, &ThisClass::HandleStartGameUnhovered);
	Button_LoadGame->OnHovered.RemoveDynamic(this, &ThisClass::HandleLoadGameHovered);
	Button_LoadGame->OnUnhovered.RemoveDynamic(this, &ThisClass::HandleLoadGameUnhovered);
	Button_Setting->OnHovered.RemoveDynamic(this, &ThisClass::HandleSettingHovered);
	Button_Setting->OnUnhovered.RemoveDynamic(this, &ThisClass::HandleSettingUnhovered);
	Button_QuitGame->OnHovered.RemoveDynamic(this, &ThisClass::HandleQuitGameHovered);
	Button_QuitGame->OnUnhovered.RemoveDynamic(this, &ThisClass::HandleQuitGameUnhovered);
	if (Button_DebugDay8)
	{
		Button_DebugDay8->OnClicked.RemoveDynamic(this, &ThisClass::HandleDebugDay8Clicked);
	}

	Super::NativeDestruct();
}

void UECMainMenuWidget::SetBackLightVisible(UImage* BackLight, const bool bVisible)
{
	BackLight->SetVisibility(
		bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UECMainMenuWidget::HandleStartGameHovered()
{
	SetBackLightVisible(Image_StartGameBackLight, true);
}

void UECMainMenuWidget::HandleStartGameUnhovered()
{
	SetBackLightVisible(Image_StartGameBackLight, false);
}

void UECMainMenuWidget::HandleLoadGameHovered()
{
	SetBackLightVisible(Image_LoadGameBackLight, true);
}

void UECMainMenuWidget::HandleLoadGameUnhovered()
{
	SetBackLightVisible(Image_LoadGameBackLight, false);
}

void UECMainMenuWidget::HandleSettingHovered()
{
	SetBackLightVisible(Image_SettingBackLight, true);
}

void UECMainMenuWidget::HandleSettingUnhovered()
{
	SetBackLightVisible(Image_SettingBackLight, false);
}

void UECMainMenuWidget::HandleQuitGameHovered()
{
	SetBackLightVisible(Image_QuitGameBackLight, true);
}

void UECMainMenuWidget::HandleQuitGameUnhovered()
{
	SetBackLightVisible(Image_QuitGameBackLight, false);
}

void UECMainMenuWidget::HandleDebugDay8Clicked()
{
	if (UECGameInstance* ECGameInstance = Cast<UECGameInstance>(GetGameInstance()))
	{
		ECGameInstance->StartDay8MorningDebugGame();
	}
}
