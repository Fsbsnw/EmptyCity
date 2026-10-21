#include "UI/Widget/TradingPost/ECTradingItemDetailWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/Widget/Common/Item/ECItemQuantitySelectorWidget.h"

void UECTradingItemDetailWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Button_ConfirmTrade->OnClicked.RemoveDynamic(this, &ThisClass::HandleConfirmClicked);
	Button_ConfirmTrade->OnClicked.AddDynamic(this, &ThisClass::HandleConfirmClicked);
	ItemQuantitySelector->OnQuantityChanged.RemoveAll(this);
	ItemQuantitySelector->OnQuantityChanged.AddUObject(
		this,
		&ThisClass::HandleQuantityChanged);
	RefreshConfirmButton();
}

void UECTradingItemDetailWidget::NativeDestruct()
{
	Button_ConfirmTrade->OnClicked.RemoveDynamic(this, &ThisClass::HandleConfirmClicked);
	ItemQuantitySelector->OnQuantityChanged.RemoveAll(this);
	OnConfirmRequested.Clear();
	OnQuantityChanged.Clear();

	Super::NativeDestruct();
}

void UECTradingItemDetailWidget::SetConfirmAvailable(bool bNewConfirmAvailable)
{
	bConfirmAvailable = bNewConfirmAvailable;
	RefreshConfirmButton();
}

void UECTradingItemDetailWidget::SetMaximumQuantity(int32 NewMaximumQuantity)
{
	ItemQuantitySelector->SetMaximumQuantity(NewMaximumQuantity);
}

void UECTradingItemDetailWidget::ResetQuantityForSelection(int32 NewMaximumQuantity)
{
	ItemQuantitySelector->ResetForSelection(NewMaximumQuantity);
}

int32 UECTradingItemDetailWidget::GetSelectedQuantity() const
{
	return ItemQuantitySelector->GetSelectedQuantity();
}

void UECTradingItemDetailWidget::SetTradePreviewValues(
	int64 TotalTradePrice,
	int64 CoinBeforeTrade,
	int64 CoinAfterTrade,
	float BagWeightBeforeTrade,
	float BagWeightAfterTrade)
{
	FNumberFormattingOptions WeightFormatting;
	WeightFormatting.MinimumFractionalDigits = 0;
	WeightFormatting.MaximumFractionalDigits = 2;

	Text_CoinBeforeTrade->SetText(FText::AsNumber(CoinBeforeTrade));
	Text_CoinAfterTrade->SetText(FText::AsNumber(CoinAfterTrade));
	if (Text_ItemPrice_Secondary)
	{
		Text_ItemPrice_Secondary->SetText(
			FText::AsNumber(FMath::Max<int64>(0, TotalTradePrice)));
	}
	Text_BagWeightBeforeTrade->SetText(
		FText::AsNumber(FMath::Max(0.0f, BagWeightBeforeTrade), &WeightFormatting));
	Text_BagWeightAfterTrade->SetText(
		FText::AsNumber(FMath::Max(0.0f, BagWeightAfterTrade), &WeightFormatting));
}

void UECTradingItemDetailWidget::ClearTradeItem()
{
	ClearItem();
	ResetQuantityForSelection(0);
	SetConfirmAvailable(false);
	Text_CoinBeforeTrade->SetText(FText::GetEmpty());
	Text_CoinAfterTrade->SetText(FText::GetEmpty());
	if (Text_ItemPrice_Secondary)
	{
		Text_ItemPrice_Secondary->SetText(FText::GetEmpty());
	}
	Text_BagWeightBeforeTrade->SetText(FText::GetEmpty());
	Text_BagWeightAfterTrade->SetText(FText::GetEmpty());
}

void UECTradingItemDetailWidget::HandleConfirmClicked()
{
	if (bConfirmAvailable)
	{
		OnConfirmRequested.Broadcast();
	}
}

void UECTradingItemDetailWidget::HandleQuantityChanged(int32 NewQuantity)
{
	OnQuantityChanged.Broadcast(NewQuantity);
}

void UECTradingItemDetailWidget::RefreshConfirmButton()
{
	if (!Button_ConfirmTrade)
	{
		return;
	}

	Button_ConfirmTrade->SetIsEnabled(true);
	Button_ConfirmTrade->SetRenderOpacity(
		bConfirmAvailable ? 1.0f : UnavailableConfirmButtonOpacity);
}
