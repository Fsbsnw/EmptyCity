// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Inventory/ECInventoryWidget.h"

#include "ECInventorySlotWidget.h"
#include "Components/WrapBox.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "UI/ViewModel/InventoryViewModel.h"

void UECInventoryWidget::InjectViewModel(UMVVMViewModelBase* TargetViewModel)
{
	Super::InjectViewModel(TargetViewModel);

	// 인벤토리 뷰모델에서 변화를 감지하도록 바인딩합니다.
	UInventoryViewModel* InventoryViewModel = GetViewModel<UInventoryViewModel>();
	if (InventoryViewModel)
	{
		InventoryViewModel->OnVMInventoryUpdated.AddUObject(this, &ThisClass::RefreshInventoryUI);
	}
}

void UECInventoryWidget::RefreshInventoryUI()
{
	UInventoryViewModel* InventoryViewModel = GetViewModel<UInventoryViewModel>();
	if (!InventoryViewModel || !InventoryWrapBox || !SlotWidgetClass) return;

	const int32 MaxFixedSlots = 20;

	// 머지 후: 실제 인벤토리 데이터를 순회하며 슬롯 생성
	const TArray<FInventoryEntry>& Entries = InventoryViewModel->GetInventoryEntries();

	// 1. 껍데기가 없으면 최초 20개 생성
	if (InventoryWrapBox->GetChildrenCount() == 0)
	{
		for (int32 i = 0; i < MaxFixedSlots; ++i)
		{
			UECInventorySlotWidget* SlotWidget = CreateWidget<UECInventorySlotWidget>(this, SlotWidgetClass);
			InventoryWrapBox->AddChildToWrapBox(SlotWidget);
		}
	}

	// 2. 당겨온 데이터(Entries)를 바탕으로 내용물만 덮어쓰기
	for (int32 i = 0; i < MaxFixedSlots; ++i)
	{
		if (UECInventorySlotWidget* SlotWidget = Cast<UECInventorySlotWidget>(InventoryWrapBox->GetChildAt(i)))
		{
			if (i < Entries.Num())
			{
				// 데이터가 있는 칸
				SlotWidget->RefreshSlotUI(Entries[i].Instance, Entries[i].StackCount);
			}
			else
			{
				// 데이터가 없는 칸 (빈 슬롯 이미지 처리)
				SlotWidget->SetEmptySlot();
			}
		}
	}
}
