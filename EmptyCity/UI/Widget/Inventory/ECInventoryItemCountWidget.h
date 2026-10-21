#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECInventoryItemCountWidget.generated.h"

class UECInventoryItemDefinition;
class UInventoryItemCountViewModel;

/**
 * 한 종류의 인벤토리 아이템 수량을 표시하는 재사용 위젯입니다.
 * 파생 WBP가 UInventoryItemCountViewModel.ItemCount 또는 ItemCountText를 원하는 UI에 바인딩합니다.
 */
UCLASS()
class EMPTYCITY_API UECInventoryItemCountWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory|Item Count")
	void SetItemDefinition(TSubclassOf<UECInventoryItemDefinition> NewItemDefinition);

	UFUNCTION(BlueprintPure, Category = "Inventory|Item Count")
	TSubclassOf<UECInventoryItemDefinition> GetItemDefinition() const { return ItemDefinition; }

protected:
	virtual void NativeConstruct() override;

	/** 이 위젯 인스턴스가 표시할 아이템 종류입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Item Count", meta = (ExposeOnSpawn = "true"))
	TSubclassOf<UECInventoryItemDefinition> ItemDefinition;

private:
	UPROPERTY(Transient)
	TObjectPtr<UInventoryItemCountViewModel> ItemCountViewModel;
};
