// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECInventoryWidget.generated.h"

class UECInventorySlotWidget;
class UWrapBox;
class UECInventoryManagerComponent;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECInventoryWidget : public UECUserWidget
{
	GENERATED_BODY()
public:
	virtual void InjectViewModel(UMVVMViewModelBase* TargetViewModel) override;
	
	/** 인벤토리가 변경될 때마다 위젯을 업데이트 합니다. */
	void RefreshInventoryUI();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> InventoryWrapBox;

	/** 인벤토리 슬롯 위젯에 사용할 블루프린트 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UECInventorySlotWidget> SlotWidgetClass;
};
