#include "UI/Widget/Map/ECRegionVisitLogWidget.h"

#include "Components/Image.h"

void UECRegionVisitLogWidget::SetVisitRecord(
	const FECRegionVisitRecord& NewVisitRecord,
	UTexture2D* RegionImage)
{
	VisitRecord = NewVisitRecord;

	if (Image_Region)
	{
		Image_Region->SetBrushFromTexture(RegionImage, false);
	}
}
