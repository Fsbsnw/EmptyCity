#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECDayCountWidget.generated.h"

/** DayProgress에서 한 날짜가 어떤 진행 상태인지 나타냅니다. */
UENUM(BlueprintType)
enum class EECDayProgressState : uint8
{
	Passed		UMETA(DisplayName = "지난 날짜"),
	Today		UMETA(DisplayName = "오늘"),
	Upcoming	UMETA(DisplayName = "아직 도달하지 않은 날짜")
};

/** DayProgress를 구성하는 날짜 한 칸의 표시 전용 위젯입니다. */
UCLASS()
class EMPTYCITY_API UECDayCountWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	/** 부모 DayProgress가 계산한 날짜 번호와 상태를 위젯 블루프린트에 전달합니다. */
	void UpdateDayState(int32 InDayNumber, EECDayProgressState NewState);

	UFUNCTION(BlueprintPure, Category = "Day Progress")
	int32 GetDayNumber() const { return DayNumber; }

	UFUNCTION(BlueprintPure, Category = "Day Progress")
	EECDayProgressState GetDayState() const { return DayState; }

	/** WBP_DayCount는 이 이벤트에서 이미지·색상·애니메이션만 꾸밉니다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Day Progress", meta = (DisplayName = "Update Day Visual"))
	void UpdateDayVisual(int32 InDayNumber, EECDayProgressState NewState);

private:
	UPROPERTY(BlueprintReadOnly, Category = "Day Progress", meta = (AllowPrivateAccess = "true"))
	int32 DayNumber = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Day Progress", meta = (AllowPrivateAccess = "true"))
	EECDayProgressState DayState = EECDayProgressState::Upcoming;
};
