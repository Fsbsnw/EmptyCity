#include "UI/Widget/DayProgress/ECDayProgressWidget.h"

#include "Components/UniformGridPanel.h"
#include "Subsystem/ECTimeSubsystem.h"
#include "UI/Widget/DayProgress/ECDayCountWidget.h"

void UECDayProgressWidget::SetCurrentDay(int32 NewCurrentDay)
{
	const int32 ClampedDay = FMath::Clamp(NewCurrentDay, UECTimeSubsystem::MinDay, UECTimeSubsystem::MaxDay);

	if (CurrentDay != ClampedDay)
	{
		CurrentDay = ClampedDay;
		RefreshDayStates();
	}
}

void UECDayProgressWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	RefreshDayStates();
}

void UECDayProgressWidget::RefreshDayStates()
{
	if (!UniformGridPanel)
	{
		return;
	}

	int32 DayNumber = 0;

	for (int32 ChildIndex = 0; ChildIndex < UniformGridPanel->GetChildrenCount(); ++ChildIndex)
	{
		if (UECDayCountWidget* DayCountWidget =	Cast<UECDayCountWidget>(UniformGridPanel->GetChildAt(ChildIndex)))
		{
			++DayNumber;

			const EECDayProgressState NewState =
				DayNumber < CurrentDay ? EECDayProgressState::Passed
				: DayNumber == CurrentDay ? EECDayProgressState::Today
				: EECDayProgressState::Upcoming;

			DayCountWidget->UpdateDayState(DayNumber, NewState);
		}
	}
}
