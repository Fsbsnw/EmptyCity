#include "UI/Widget/ECSystemMenuWidget.h"

#include "ECGameplayTags.h"
#include "Components/Button.h"
#include "Core/GameInstance/ECGameInstance.h"
#include "Engine/GameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Save/ECSaveSubsystem.h"
#include "UI/Subsystem/UIManagerSubsystem.h"

void UECSystemMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SaveButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleSaveClicked);
	LoadButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleLoadClicked);

	// WBP_SystemMenu에 남아 있는 기존 설정 버튼 이벤트가 위젯을 중복 생성하지 않도록 제거합니다.
	SettingsButton->OnClicked.RemoveAll(this);
	SettingsButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleSettingsClicked);

	MainMenuButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleMainMenuClicked);
	QuitButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleQuitClicked);
	if (DebugDay8Button)
	{
		DebugDay8Button->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleDebugDay8Clicked);
	}
}

void UECSystemMenuWidget::HandleSaveClicked()
{
	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UECSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UECSaveSubsystem>())
		{
			SaveSubsystem->SaveCurrentGame();
		}
	}
}

void UECSystemMenuWidget::HandleLoadClicked()
{
	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UECSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UECSaveSubsystem>())
		{
			SaveSubsystem->LoadCurrentGame();
		}
	}
}

void UECSystemMenuWidget::HandleSettingsClicked()
{
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(GetOwningPlayer()))
	{
		UIManager->RequestToggleInput(ECGameplayTags::InputTag_UI_SettingMenu);
	}
}

void UECSystemMenuWidget::HandleMainMenuClicked()
{
}

void UECSystemMenuWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(
		this,
		GetOwningPlayer(),
		EQuitPreference::Quit,
		false);
}

void UECSystemMenuWidget::HandleDebugDay8Clicked()
{
	if (UECGameInstance* ECGameInstance = Cast<UECGameInstance>(GetGameInstance()))
	{
		ECGameInstance->StartDay8MorningDebugGame();
	}
}
