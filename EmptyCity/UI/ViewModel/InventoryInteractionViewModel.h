// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "InventoryInteractionViewModel.generated.h"

class UECInventoryManagerComponent;
class UECInventoryItemInstance;
class UInventoryViewModel;
class AECInventoryContainer;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UInventoryInteractionViewModel : public UECViewModelBase
{
	GENERATED_BODY()
	
public:
	virtual void BindCallbacksToDependencies(AActor* ContextActor) override;
	virtual void BroadcastInitialValues() override;
	
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory")
	UInventoryViewModel* GetPlayerInventoryViewModel() const
	{
		return PlayerInventoryViewModel;
	}

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory")
	UInventoryViewModel* GetStorageInventoryViewModel() const
	{
		return StorageInventoryViewModel;
	}

	bool GetCanTransferItems() const;

	int32 MoveAllItemsToStorage();
	int32 MoveItemToStorage(UECInventoryItemInstance* ItemInstance,	int32 Count = 1);
	int32 MoveItemToPlayer(UECInventoryItemInstance* ItemInstance, int32 Count = 1);

	int32 DiscardPlayerItem(UECInventoryItemInstance* ItemInstance,	int32 Count = 1);

	/** 현재 상호작용 중인 보관함/교환대 액터를 반환합니다. */
	AECInventoryContainer* GetStorageContainer() const;

private:
	UPROPERTY()
	TObjectPtr<UInventoryViewModel>	PlayerInventoryViewModel;

	UPROPERTY()
	TObjectPtr<UInventoryViewModel>	StorageInventoryViewModel;

	UPROPERTY()
	TWeakObjectPtr<UECInventoryManagerComponent> PlayerInventoryComponent;

	UPROPERTY()
	TWeakObjectPtr<UECInventoryManagerComponent> StorageInventoryComponent;
};
