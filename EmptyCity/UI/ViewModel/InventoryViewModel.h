// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "InventoryViewModel.generated.h"

struct FInventoryEntry;
DECLARE_MULTICAST_DELEGATE(FOnVMInventoryUpdatedSignature);

class UECInventoryManagerComponent;
/**
 * 
 */
UCLASS()
class EMPTYCITY_API UInventoryViewModel : public UECViewModelBase
{
	GENERATED_BODY()
	
public:
	virtual void BindCallbacksToDependencies(AActor* ContextActor) override;
	virtual void BroadcastInitialValues() override;

	/** Inventory Entries들을 읽어오는 함수입니다. (위젯이랑 바인딩) */
	const TArray<FInventoryEntry>& GetInventoryEntries() const;
	
	/** 위젯이 바인딩할 델리게이트입니다. */
	FOnVMInventoryUpdatedSignature OnVMInventoryUpdated;

private:
	/** 인벤토리에 바인딩 하는 함수입니다. */
	UFUNCTION()
	void OnInventoryUpdated();
	
	/** 인벤토리를 캐싱하여 관리합니다. */
	UPROPERTY()
	TObjectPtr<UECInventoryManagerComponent> CachedInventory;
};
