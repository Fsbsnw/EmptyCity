#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "InventoryItemCountViewModel.generated.h"

class UECInventoryItemDefinition;
class UECInventoryManagerComponent;

/** 플레이어 인벤토리에서 지정한 Item Definition의 총 수량을 제공하는 범용 ViewModel입니다. */
UCLASS()
class EMPTYCITY_API UInventoryItemCountViewModel : public UECViewModelBase
{
	GENERATED_BODY()

public:
	/** 플레이어 인벤토리와 추적할 아이템 종류를 지정합니다. */
	void Initialize(AActor* ContextActor, TSubclassOf<UECInventoryItemDefinition> InItemDefinition);

	virtual void BroadcastInitialValues() override;
	virtual void BeginDestroy() override;

	int32 GetItemCount() const { return ItemCount; }
	void SetItemCount(int32 NewItemCount);

	/** TextBlock.Text에 변환 함수 없이 바인딩할 수 있는 수량 텍스트입니다. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory|Item Count")
	FText GetItemCountText() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Item Count")
	TSubclassOf<UECInventoryItemDefinition> GetItemDefinition() const { return ItemDefinition; }

	/** 같은 ViewModel 인스턴스가 추적할 아이템 종류를 런타임에 변경합니다. */
	void SetItemDefinition(TSubclassOf<UECInventoryItemDefinition> NewItemDefinition);

private:
	void HandleInventoryUpdated();
	void RefreshItemCount();

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"))
	int32 ItemCount = 0;

	UPROPERTY(BlueprintReadOnly, Getter, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UECInventoryItemDefinition> ItemDefinition;

	UPROPERTY()
	TWeakObjectPtr<UECInventoryManagerComponent> InventoryComponent;
};
