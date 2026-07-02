// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/InventoryViewModel.h"

#include "Character/Player/Controller/ECPlayerController.h"
#include "Inventory/ECInventoryManagerComponent.h"

void UInventoryViewModel::BindCallbacksToDependencies(AActor* ContextActor)
{
	if (!ContextActor) return;

	// 1. 넘어온 액터를 인터페이스로 직접 캐스팅
	if (IIECInventory* InventoryInterface = Cast<IIECInventory>(ContextActor))
	{
		CachedInventory = InventoryInterface->GetInventoryManagerComponent();
	}
	// 2. Pawn인 경우 컨트롤러를 검사
	else if (APawn* ContextPawn = Cast<APawn>(ContextActor))
	{
		if (AController* PC = ContextPawn->GetController())
		{
			// 컨트롤러도 인터페이스 캐스팅 검사
			if (IIECInventory* PCInterface = Cast<IIECInventory>(PC))
			{
				CachedInventory = PCInterface->GetInventoryManagerComponent();
			}
		}
	}

	// 3. 최종 델리게이트 바인딩
	if (CachedInventory)
	{
		CachedInventory->OnInventoryUpdated.AddUObject(this, &ThisClass::OnInventoryUpdated);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("인벤토리 인터페이스를 찾을 수 없습니다: %s"), *ContextActor->GetName());
	}
}

void UInventoryViewModel::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();
	
	OnVMInventoryUpdated.Broadcast();
}

const TArray<FInventoryEntry>& UInventoryViewModel::GetInventoryEntries() const
{
	if (CachedInventory)
	{
		return CachedInventory->InventoryList.Entries;
	}
	// 컴포넌트 없는 경우 대비
	static TArray<FInventoryEntry> EmptyEntries;
	return EmptyEntries;
}

void UInventoryViewModel::OnInventoryUpdated()
{
	OnVMInventoryUpdated.Broadcast();
}
