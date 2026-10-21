#include "UI/Widget/ECPlayerDeathWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "UI/ViewModel/PlayerDeathViewModel.h"

void UECPlayerDeathWidget::InjectViewModel(UMVVMViewModelBase* TargetViewModel)
{
	Super::InjectViewModel(TargetViewModel);

	const UPlayerDeathViewModel* DeathViewModel = Cast<UPlayerDeathViewModel>(TargetViewModel);
	if (!DeathViewModel)
	{
		return;
	}

	UTexture2D* DeathImage = DeathViewModel->GetDeathScreenImage();
	if (Image_Death && DeathImage)
	{
		Image_Death->SetBrushFromTexture(DeathImage, false);
	}

	if (Image_LostItem)
	{
		Image_LostItem->SetBrushFromTexture(
			DeathViewModel->GetDroppedItemIcon(),
			false);
	}
	if (Text_DroppedItemName)
	{
		Text_DroppedItemName->SetText(DeathViewModel->GetDroppedItemName());
	}
	if (Text_DroppedItemCount)
	{
		const int32 DroppedCount = DeathViewModel->GetDroppedItemCount();
		Text_DroppedItemCount->SetText(
			DroppedCount > 0 ? FText::AsNumber(DroppedCount) : FText::GetEmpty());
	}
}
