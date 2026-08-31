// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "InventoryInteractionViewModel.generated.h"

class UECInventoryManagerComponent;
class UECInventoryItemInstance;
class UInventoryViewModel;
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
	int32 DiscardStorageItem(UECInventoryItemInstance* ItemInstance, int32 Count = 1);

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