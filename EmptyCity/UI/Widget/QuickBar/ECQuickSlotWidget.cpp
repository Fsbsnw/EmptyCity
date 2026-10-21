#include "UI/Widget/QuickBar/ECQuickSlotWidget.h"

void UECQuickSlotWidget::SetSlotData(const FECQuickSlotViewData& NewSlotData)
{
	SlotData = NewSlotData;
	UpdateQuickSlotVisual(SlotData);
}
