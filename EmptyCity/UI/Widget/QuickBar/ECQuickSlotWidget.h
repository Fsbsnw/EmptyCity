#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "UI/Widget/QuickBar/ECQuickSlotTypes.h"
#include "ECQuickSlotWidget.generated.h"

/** ViewModel에서 받은 슬롯 데이터를 WBP의 시각 연출 이벤트로 전달하는 범용 위젯입니다. */
UCLASS()
class EMPTYCITY_API UECQuickSlotWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	FECQuickSlotViewData GetSlotData() const { return SlotData; }
	void SetSlotData(const FECQuickSlotViewData& NewSlotData);

protected:
	/** WBP_QuickSlot은 이 이벤트에서 아이콘, 재고 텍스트, 비활성 표현만 처리합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Quick Slot", meta = (DisplayName = "Update Quick Slot Visual"))
	void UpdateQuickSlotVisual(const FECQuickSlotViewData& NewSlotData);

private:
	UPROPERTY(BlueprintReadWrite, Setter, Getter, meta = (AllowPrivateAccess = "true"))
	FECQuickSlotViewData SlotData;
};
