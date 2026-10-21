#include "UI/Widget/Tutorial/ECTutorialWidget.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Data/Tutorial/ECTutorialSettings.h"
#include "Engine/Texture2D.h"

void UECTutorialWidget::ApplyTutorialState(
	const EECTutorialStage NewStage,
	const bool bNewPrimaryActionCompleted,
	const bool bNewSecondaryActionCompleted)
{
	CurrentStage = NewStage;
	bPrimaryActionCompleted = bNewPrimaryActionCompleted;
	bSecondaryActionCompleted = bNewSecondaryActionCompleted;

	const bool bShouldDisplay =
		CurrentStage == EECTutorialStage::Movement
		|| CurrentStage == EECTutorialStage::OpenMap
		|| CurrentStage == EECTutorialStage::Sprint
		|| CurrentStage == EECTutorialStage::AttackAndGuard;
	SetVisibility(bShouldDisplay
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);

	if (Image_Tutorial)
	{
		UTexture2D* TutorialTexture = nullptr;
		const UECTutorialSettings* Settings = GetDefault<UECTutorialSettings>();
		if (Settings)
		{
			if (const TSoftObjectPtr<UTexture2D>* TextureReference =
				Settings->TutorialImages.Find(CurrentStage))
			{
				TutorialTexture = TextureReference->LoadSynchronous();
			}
		}

		Image_Tutorial->SetBrushFromTexture(TutorialTexture, true);
		Image_Tutorial->SetVisibility(TutorialTexture
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::Collapsed);
		ApplyTutorialImageLayout(Settings);
	}

	OnTutorialStateChanged(
		CurrentStage,
		bPrimaryActionCompleted,
		bSecondaryActionCompleted);
}

void UECTutorialWidget::ApplyTutorialImageLayout(const UECTutorialSettings* Settings)
{
	if (!Image_Tutorial)
	{
		return;
	}

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Image_Tutorial->Slot);
	if (!CanvasSlot)
	{
		return;
	}

	if (!bHasCachedDefaultImageLayout)
	{
		bHasCachedDefaultImageLayout = true;
		bDefaultImageAutoSize = CanvasSlot->GetAutoSize();
		DefaultImagePosition = CanvasSlot->GetPosition();
		DefaultImageSize = CanvasSlot->GetSize();
	}

	const FECTutorialImageLayout* Layout = Settings
		? Settings->TutorialImageLayouts.Find(CurrentStage)
		: nullptr;

	CanvasSlot->SetPosition(Layout && Layout->bOverridePosition
		? Layout->Position
		: DefaultImagePosition);

	if (Layout && Layout->bOverrideSize)
	{
		CanvasSlot->SetAutoSize(false);
		CanvasSlot->SetSize(Layout->Size);
	}
	else
	{
		CanvasSlot->SetAutoSize(bDefaultImageAutoSize);
		CanvasSlot->SetSize(DefaultImageSize);
	}
}

