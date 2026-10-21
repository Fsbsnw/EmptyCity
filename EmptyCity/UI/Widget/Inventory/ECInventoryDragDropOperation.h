#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "ECInventoryDragDropOperation.generated.h"

class UECInventoryItemInstance;

/** 보관함 드래그가 시작된 인벤토리 영역입니다. 슬롯 위치 정보는 보관하지 않습니다. */
UENUM()
enum class EECInventoryDragSource : uint8
{
	None,
	Player,
	Storage
};

/** 플레이어 인벤토리와 보관함 사이의 단순 아이템 이동에만 사용하는 드래그 데이터입니다. */
UCLASS()
class EMPTYCITY_API UECInventoryDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	void Initialize(
		UECInventoryItemInstance* InItemInstance,
		EECInventoryDragSource InSource,
		int32 InStackCount)
	{
		ItemInstance = InItemInstance;
		Source = InSource;
		StackCount = FMath::Max(1, InStackCount);
	}

	UECInventoryItemInstance* GetItemInstance() const { return ItemInstance; }
	EECInventoryDragSource GetSource() const { return Source; }
	int32 GetStackCount() const { return StackCount; }

private:
	UPROPERTY(Transient)
	TObjectPtr<UECInventoryItemInstance> ItemInstance;

	EECInventoryDragSource Source = EECInventoryDragSource::None;
	int32 StackCount = 1;
};
