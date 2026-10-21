#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ECInvincibilityIndicatorWidget.generated.h"

/** F5 무적 상태를 표시하는 간단한 코드 전용 위젯입니다. */
UCLASS()
class EMPTYCITY_API UECInvincibilityIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
