#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ECTutorialTypes.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECTutorialWidget.generated.h"

class UImage;
class UCanvasPanelSlot;
class UECTutorialSettings;

/** 튜토리얼 상태를 이미지와 블루프린트 연출로 표시합니다. */
UCLASS(Abstract, Blueprintable)
class EMPTYCITY_API UECTutorialWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	void ApplyTutorialState(
		EECTutorialStage NewStage,
		bool bNewPrimaryActionCompleted,
		bool bNewSecondaryActionCompleted);

protected:
	/** 단계 또는 복합 입력 진행도가 바뀔 때 WBP의 보조 이미지와 애니메이션을 갱신합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Tutorial", meta=(DisplayName="On Tutorial State Changed"))
	void OnTutorialStateChanged(
		EECTutorialStage NewStage,
		bool bPrimaryCompleted,
		bool bSecondaryCompleted);

	/** Tutorial Settings에 등록한 단계별 대표 이미지를 표시합니다. */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UImage> Image_Tutorial;

	UPROPERTY(BlueprintReadOnly, Category="Tutorial")
	EECTutorialStage CurrentStage = EECTutorialStage::Movement;

	/** Shift/공격처럼 해당 단계의 첫 번째 행동 완료 여부입니다. */
	UPROPERTY(BlueprintReadOnly, Category="Tutorial")
	bool bPrimaryActionCompleted = false;

	/** 공격/방어 단계의 두 번째 행동인 방어 완료 여부입니다. */
	UPROPERTY(BlueprintReadOnly, Category="Tutorial")
	bool bSecondaryActionCompleted = false;

private:
	/** 현재 단계의 설정을 적용하고, 미설정 값은 WBP의 최초 배치로 복원합니다. */
	void ApplyTutorialImageLayout(const UECTutorialSettings* Settings);

	bool bHasCachedDefaultImageLayout = false;
	bool bDefaultImageAutoSize = false;
	FVector2D DefaultImagePosition = FVector2D::ZeroVector;
	FVector2D DefaultImageSize = FVector2D::ZeroVector;
};

