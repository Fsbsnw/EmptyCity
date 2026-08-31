// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "InventoryViewModel.generated.h"

class UECInventoryItemInstance;
class UECInventoryManagerComponent;
struct FInventoryEntry;

DECLARE_MULTICAST_DELEGATE(FOnVMInventoryUpdatedSignature);

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UInventoryViewModel : public UECViewModelBase
{
	GENERATED_BODY()
	
public:
	void Initialize(UECInventoryManagerComponent* InInventoryComponent);
	virtual void BroadcastInitialValues() override;
	virtual void BeginDestroy() override;

	/** Inventory Entries들을 읽어오는 함수입니다. (위젯이랑 바인딩) */
	const TArray<FInventoryEntry>& GetInventoryEntries() const;

	/** 특정 아이템 인스턴스가 현재 인벤토리에 남아 있는지 확인합니다. */
	bool ContainsItem(UECInventoryItemInstance* ItemInstance) const;
	
	/** 유효한 인벤토리가 연결돼 있는지 반환합니다. */
	bool HasInventory() const;
	
	/** 위젯이 바인딩할 델리게이트입니다. */
	FOnVMInventoryUpdatedSignature OnVMInventoryUpdated;

private:
	/** 인벤토리에 바인딩 하는 함수입니다. */
	UFUNCTION()
	void OnInventoryUpdated();
	
	/**
	 * 이 ViewModel이 표시할 인벤토리입니다.
	 *
	 * ViewModel이 컴포넌트의 생명주기를 소유하지 않으므로
	 * WeakPtr로 참조합니다.
	 */
	UPROPERTY()
	TWeakObjectPtr<UECInventoryManagerComponent> InventoryComponent;
};
