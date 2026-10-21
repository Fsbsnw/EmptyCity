#include "UI/Widget/DayProgress/ECDayCountWidget.h"

void UECDayCountWidget::UpdateDayState(int32 InDayNumber, EECDayProgressState NewState)
{
	DayNumber = FMath::Max(1, InDayNumber);
	DayState = NewState;

	// 어떤 이미지와 연출을 사용할지는 WBP_DayCount가 전담합니다.
	UpdateDayVisual(DayNumber, DayState);
}
