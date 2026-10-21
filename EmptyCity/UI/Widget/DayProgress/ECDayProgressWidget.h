#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECDayProgressWidget.generated.h"

class UECDayCountWidget;
class UUniformGridPanel;

/** 현재 날짜를 받아 10개의 DayCount 표시 상태를 일괄 갱신하는 위젯입니다. */
UCLASS()
class EMPTYCITY_API UECDayProgressWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	int32 GetCurrentDay() const { return CurrentDay; }
	void SetCurrentDay(int32 NewCurrentDay);

protected:
	virtual void NativePreConstruct() override;

private:
	void RefreshDayStates();

private:
	/** WBP_HUD에서 TimeViewModel.CurrentDay를 이 속성으로 단방향 바인딩합니다. */
	UPROPERTY(BlueprintReadWrite, Setter, Getter, meta = (AllowPrivateAccess = "true"))
	int32 CurrentDay = 1;

protected:
	/** WBP_DayProgress에 배치된 DayCount들을 담는 UniformGrid입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> UniformGridPanel;
};
