#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ECItemQuantitySelectorWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemQuantityChanged, int32);

/**
 * 버튼으로 1~MaximumQuantity 범위의 아이템 작업 수량을 선택하는 공통 위젯입니다.
 * MaximumQuantity가 0이면 자동으로 숨깁니다.
 */
UCLASS()
class EMPTYCITY_API UECItemQuantitySelectorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** 현재 선택을 유지하면서 최대 수량을 변경합니다. */
	void SetMaximumQuantity(int32 NewMaximumQuantity);

	/** 새 아이템을 선택했을 때 최대 수량을 적용하고 1개로 초기화합니다. */
	void ResetForSelection(int32 NewMaximumQuantity);

	UFUNCTION(BlueprintPure, Category = "Item Quantity")
	int32 GetSelectedQuantity() const { return SelectedQuantity; }

	UFUNCTION(BlueprintPure, Category = "Item Quantity")
	int32 GetMaximumQuantity() const { return MaximumQuantity; }

	FOnItemQuantityChanged OnQuantityChanged;

private:
	void SetSelectedQuantity(int32 NewSelectedQuantity, bool bForceNotify = false);
	void RefreshQuantityUI();

	UFUNCTION()
	void HandleDecreaseClicked();

	UFUNCTION()
	void HandleIncreaseClicked();

	UFUNCTION()
	void HandleSelectAllClicked();

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_DecreaseQuantity;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_IncreaseQuantity;

	/** 현재 아이템의 전체 수량을 선택합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_SelectAllQuantity;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_SelectedQuantity;

	/**
	 * 최소/최대 수량에 도달한 버튼의 표시 투명도입니다.
	 * 버튼을 Disable하면 클릭이 부모 위젯으로 전달되므로, 활성 상태를 유지하고 시각만 구분합니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Quantity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UnavailableButtonOpacity = 0.35f;

private:
	int32 SelectedQuantity = 0;
	int32 MaximumQuantity = 0;
};
