#pragma once

#include "CoreMinimal.h"
#include "Save/ECSaveTypes.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECRegionVisitLogWidget.generated.h"

class UImage;
class UTexture2D;

/**
 * 방문 기록 한 건을 표시하는 위젯입니다.
 * 레이아웃과 스타일은 파생 WBP가 담당하고, C++은 지역 이미지 바인딩만 처리합니다.
 */
UCLASS()
class EMPTYCITY_API UECRegionVisitLogWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	/** 방문 기록과 DT_Region에서 조회한 이미지를 적용합니다. */
	void SetVisitRecord(const FECRegionVisitRecord& NewVisitRecord, UTexture2D* RegionImage);

	UFUNCTION(BlueprintPure, Category = "Region|Visit Log")
	FECRegionVisitRecord GetVisitRecord() const { return VisitRecord; }

protected:
	/** 파생 WBP에서 같은 이름의 Image 위젯을 배치해야 합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Image_Region;

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FECRegionVisitRecord VisitRecord;
};
