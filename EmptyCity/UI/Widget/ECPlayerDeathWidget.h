#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECPlayerDeathWidget.generated.h"

class UImage;
class UTextBlock;

/** PlayerDeathViewModel이 선택한 적 타입별 사망 이미지를 표시합니다. */
UCLASS()
class EMPTYCITY_API UECPlayerDeathWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	virtual void InjectViewModel(UMVVMViewModelBase* TargetViewModel) override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Image_Death;

	/** 사망 시 떨어뜨린 아이템의 아이콘입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_LostItem;

	/** 사망 시 떨어뜨린 아이템의 이름입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_DroppedItemName;

	/** 사망 시 떨어뜨린 아이템의 수량입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_DroppedItemCount;
};
