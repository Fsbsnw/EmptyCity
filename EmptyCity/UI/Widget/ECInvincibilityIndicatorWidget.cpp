#include "UI/Widget/ECInvincibilityIndicatorWidget.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UECInvincibilityIndicatorWidget::RebuildWidget()
{
	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(FMargin(24.0f))
		[
			SNew(SBorder)
			.Padding(FMargin(12.0f, 6.0f))
			.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.65f))
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("Debug", "InvincibilityEnabled", "INVINCIBLE (F5)"))
				.ColorAndOpacity(FLinearColor(1.0f, 0.85f, 0.0f, 1.0f))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
			]
		];
}
