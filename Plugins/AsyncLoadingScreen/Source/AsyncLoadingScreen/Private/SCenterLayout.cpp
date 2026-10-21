/************************************************************************************
 *																					*
 * Copyright (C) 2020 Truong Bui.													*
 * Website:	https://github.com/truong-bui/AsyncLoadingScreen						*
 * Licensed under the MIT License. See 'LICENSE' file for full license information. *
 *																					*
 ************************************************************************************/

#include "SCenterLayout.h"
#include "LoadingScreenSettings.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSafeZone.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Widgets/Layout/SScaleBox.h"
#include "SBackgroundWidget.h"
#include "STipWidget.h"

void SCenterLayout::Construct(const FArguments& InArgs, const FALoadingScreenSettings& Settings, const FCenterLayoutSettings& LayoutSettings)
{
	// Root widget and background
	TSharedRef<SOverlay> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBackgroundWidget, Settings.Background)
		];

	TArray<UTexture2D*> ValidCenterIcons;
	for (const TObjectPtr<UTexture2D>& Icon : LayoutSettings.RandomCenterIcons)
	{
		if (Icon)
		{
			ValidCenterIcons.Add(Icon.Get());
		}
	}

	if (!ValidCenterIcons.IsEmpty())
	{
		const int32 IconIndex = FMath::RandRange(0, ValidCenterIcons.Num() - 1);
		CenterIconBrush = FDeferredCleanupSlateBrush::CreateBrush(ValidCenterIcons[IconIndex]);

		// The existing Tip Widget already selects one TipText entry randomly.
		Root->AddSlot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(LayoutSettings.CenterContentPadding)
			[
				SNew(SDPIScaler)
				.DPIScale(this, &SCenterLayout::GetDPIScale)
				[
					SNew(SBorder)
					.BorderImage(&LayoutSettings.BorderBackground)
					.BorderBackgroundColor(FLinearColor::White)
					.Padding(LayoutSettings.BorderPadding)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(LayoutSettings.CenterIconSize.X)
							.HeightOverride(LayoutSettings.CenterIconSize.Y)
							[
								SNew(SScaleBox)
								.Stretch(EStretch::ScaleToFit)
								[
									SNew(SImage)
									.Image(CenterIconBrush->GetSlateBrush())
								]
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
						.Padding(0.0f, LayoutSettings.CenterIconTextSpacing, 0.0f, 0.0f)
						[
							SNew(STipWidget, Settings.TipWidget)
						]
					]
				]
			];

		// Random center content is decorative; keep the configured loading indicator visible as well.
		Root->AddSlot()
			.HAlign(LayoutSettings.LoadingWidgetHorizontalAlignment)
			.VAlign(LayoutSettings.LoadingWidgetVerticalAlignment)
			.Padding(LayoutSettings.LoadingWidgetPadding)
			[
				MakeLoadingWidget(Settings.LoadingWidget)
			];
	}
	else
	{
		// Preserve the original Center layout when no random center icons are configured.
		Root->AddSlot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MakeLoadingWidget(Settings.LoadingWidget)
			];

		const EVerticalAlignment TipBorderVerticalAlignment = LayoutSettings.bIsTipAtBottom ? VAlign_Bottom : VAlign_Top;
		const FMargin TipBorderOffset = LayoutSettings.bIsTipAtBottom
			? FMargin(0, 0, 0, LayoutSettings.BorderVerticalOffset)
			: FMargin(0, LayoutSettings.BorderVerticalOffset, 0, 0);

		Root->AddSlot()
			.HAlign(LayoutSettings.BorderHorizontalAlignment)
			.VAlign(TipBorderVerticalAlignment)
			.Padding(TipBorderOffset)
			[
				SNew(SBorder)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.BorderImage(&LayoutSettings.BorderBackground)
				.BorderBackgroundColor(FLinearColor::White)
				[
					SNew(SSafeZone)
					.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
					.VAlign(LayoutSettings.TipAlignment.VerticalAlignment)
					.IsTitleSafe(true)
					.Padding(LayoutSettings.BorderPadding)
					[
						SNew(SDPIScaler)
						.DPIScale(this, &SCenterLayout::GetDPIScale)
						[
							SNew(STipWidget, Settings.TipWidget)
						]
					]
				]
			];
	}

	if (LayoutSettings.BottomLeftLogo)
	{
		BottomLeftLogoBrush = FDeferredCleanupSlateBrush::CreateBrush(LayoutSettings.BottomLeftLogo.Get());

		Root->AddSlot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(LayoutSettings.BottomLeftLogoPadding)
			[
				SNew(SDPIScaler)
				.DPIScale(this, &SCenterLayout::GetDPIScale)
				[
					SNew(SBox)
					.WidthOverride(LayoutSettings.BottomLeftLogoSize.X)
					.HeightOverride(LayoutSettings.BottomLeftLogoSize.Y)
					[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFit)
						[
							SNew(SImage)
							.Image(BottomLeftLogoBrush->GetSlateBrush())
						]
					]
				]
			];
	}

	// Construct loading complete text if enable
	AddLoadingCompleteTextSlot(Root, Settings);

	// Construct PSO precache progress widget if enable
	AddPSOPrecacheProgressSlot(Root, Settings);

	// Add root to this widget
	ChildSlot
	[
		Root
	];
}
