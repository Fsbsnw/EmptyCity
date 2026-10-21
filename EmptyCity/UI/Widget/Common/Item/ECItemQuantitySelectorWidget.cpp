#include "UI/Widget/Common/Item/ECItemQuantitySelectorWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UECItemQuantitySelectorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Button_DecreaseQuantity->OnClicked.RemoveDynamic(this, &ThisClass::HandleDecreaseClicked);
	Button_DecreaseQuantity->OnClicked.AddDynamic(this, &ThisClass::HandleDecreaseClicked);
	Button_IncreaseQuantity->OnClicked.RemoveDynamic(this, &ThisClass::HandleIncreaseClicked);
	Button_IncreaseQuantity->OnClicked.AddDynamic(this, &ThisClass::HandleIncreaseClicked);
	if (Button_SelectAllQuantity)
	{
		Button_SelectAllQuantity->OnClicked.RemoveDynamic(this, &ThisClass::HandleSelectAllClicked);
		Button_SelectAllQuantity->OnClicked.AddDynamic(this, &ThisClass::HandleSelectAllClicked);
	}

	RefreshQuantityUI();
}

void UECItemQuantitySelectorWidget::SetMaximumQuantity(int32 NewMaximumQuantity)
{
	MaximumQuantity = FMath::Max(0, NewMaximumQuantity);
	SetSelectedQuantity(SelectedQuantity, true);
}

void UECItemQuantitySelectorWidget::ResetForSelection(int32 NewMaximumQuantity)
{
	MaximumQuantity = FMath::Max(0, NewMaximumQuantity);
	SetSelectedQuantity(MaximumQuantity > 0 ? 1 : 0, true);
}

void UECItemQuantitySelectorWidget::SetSelectedQuantity(int32 NewSelectedQuantity, bool bForceNotify)
{
	const int32 ClampedQuantity = MaximumQuantity > 0
		? FMath::Clamp(NewSelectedQuantity, 1, MaximumQuantity)
		: 0;
	const bool bChanged = SelectedQuantity != ClampedQuantity;
	SelectedQuantity = ClampedQuantity;
	RefreshQuantityUI();

	if (bChanged || bForceNotify)
	{
		OnQuantityChanged.Broadcast(SelectedQuantity);
	}
}

void UECItemQuantitySelectorWidget::RefreshQuantityUI()
{
	const bool bHasSelectableQuantity = MaximumQuantity > 0;
	SetVisibility(bHasSelectableQuantity ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	Text_SelectedQuantity->SetText(FText::AsNumber(SelectedQuantity));

	// Disable된 UButton은 클릭을 소비하지 않아 부모 인벤토리의 배경 클릭으로 전달됩니다.
	// 두 버튼을 활성 상태로 유지하고, 범위 끝에서는 핸들러의 Clamp와 투명도로 동작을 표현합니다.
	Button_DecreaseQuantity->SetIsEnabled(bHasSelectableQuantity);
	Button_IncreaseQuantity->SetIsEnabled(bHasSelectableQuantity);
	Button_DecreaseQuantity->SetRenderOpacity(
		SelectedQuantity > 1 ? 1.0f : UnavailableButtonOpacity);
	Button_IncreaseQuantity->SetRenderOpacity(
		SelectedQuantity < MaximumQuantity ? 1.0f : UnavailableButtonOpacity);
	if (Button_SelectAllQuantity)
	{
		Button_SelectAllQuantity->SetIsEnabled(bHasSelectableQuantity);
		Button_SelectAllQuantity->SetRenderOpacity(
			SelectedQuantity < MaximumQuantity ? 1.0f : UnavailableButtonOpacity);
	}
}

void UECItemQuantitySelectorWidget::HandleDecreaseClicked()
{
	SetSelectedQuantity(SelectedQuantity - 1);
}

void UECItemQuantitySelectorWidget::HandleIncreaseClicked()
{
	SetSelectedQuantity(SelectedQuantity + 1);
}

void UECItemQuantitySelectorWidget::HandleSelectAllClicked()
{
	SetSelectedQuantity(MaximumQuantity);
}
