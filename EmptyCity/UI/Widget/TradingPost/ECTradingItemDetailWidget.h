#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/Common/Item/ECItemDetailWidget.h"
#include "ECTradingItemDetailWidget.generated.h"

class UButton;
class UECItemQuantitySelectorWidget;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE(FOnTradingItemDetailConfirmRequested);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTradingItemDetailQuantityChanged, int32);

/**
 * 공통 아이템 정보에 교환소 전용 구매/판매 확인 버튼을 추가한 상세 위젯입니다.
 * 구매용과 판매용 WBP가 각각 이 클래스를 부모로 사용합니다.
 */
UCLASS()
class EMPTYCITY_API UECTradingItemDetailWidget : public UECItemDetailWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void SetConfirmAvailable(bool bNewConfirmAvailable);
	void SetMaximumQuantity(int32 NewMaximumQuantity);
	void ResetQuantityForSelection(int32 NewMaximumQuantity);
	int32 GetSelectedQuantity() const;
	void SetTradePreviewValues(
		int64 TotalTradePrice,
		int64 CoinBeforeTrade,
		int64 CoinAfterTrade,
		float BagWeightBeforeTrade,
		float BagWeightAfterTrade);
	void ClearTradeItem();

	FOnTradingItemDetailConfirmRequested OnConfirmRequested;
	FOnTradingItemDetailQuantityChanged OnQuantityChanged;

private:
	UFUNCTION()
	void HandleConfirmClicked();

	void HandleQuantityChanged(int32 NewQuantity);

	void RefreshConfirmButton();

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_ConfirmTrade;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UECItemQuantitySelectorWidget> ItemQuantitySelector;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_CoinBeforeTrade;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_CoinAfterTrade;

	/** Text_ItemPrice와 동일한 선택 수량 전체 구매/판매 금액을 표시합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ItemPrice_Secondary;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_BagWeightBeforeTrade;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_BagWeightAfterTrade;

	/** 거래 불가 상태에서도 버튼이 클릭을 소비하도록 활성 상태를 유지하고 투명도만 낮춥니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trade", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UnavailableConfirmButtonOpacity = 0.35f;

private:
	bool bConfirmAvailable = false;
};
