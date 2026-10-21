// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/UI/UIConfigDataAsset.h"
#include "GameFramework/Actor.h"
#include "Interaction/IInteractableTarget.h"
#include "Inventory/IECInventory.h"
#include "ECInventoryContainer.generated.h"

class UUIManagerSubsystem;
class UBoxComponent;
struct FInitialInventoryItem;

UCLASS()
class EMPTYCITY_API AECInventoryContainer : public AActor, public IIECInventory, public IInteractableTarget
{
	GENERATED_BODY()
public:
	AECInventoryContainer();

	virtual void BeginPlay() override;
	
// ─────────────────────────────────────────────────────────────
// InteractableTarget Interface
// ─────────────────────────────────────────────────────────────		
public:
	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& OptionBuilder) override;

	
// ─────────────────────────────────────────────────────────────
// Interaction
// ─────────────────────────────────────────────────────────────	
protected:
	UPROPERTY(EditAnywhere, Category = "변수|상호작용")
	FInteractionOption Option;

	/** 교환소에서 이 아이템을 보유 중이면 일반 안내 대신 제출 안내를 표시합니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|상호작용|엔딩")
	FName EndingSubmissionItemId = TEXT("Cylinder");

	/** 엔딩 제출 아이템을 보유했을 때 인디케이터에 표시할 문구입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|상호작용|엔딩")
	FText EndingSubmissionInteractionText;

// ─────────────────────────────────────────────────────────────
// Interaction Function
// ─────────────────────────────────────────────────────────────
public:	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ShowInventory(APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HideInventory(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	
// ─────────────────────────────────────────────────────────────
// Components
// ─────────────────────────────────────────────────────────────
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UStaticMeshComponent* Mesh;
    
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UBoxComponent* BoxCollision;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UECInventoryManagerComponent* InventoryManagerComponent;

// ─────────────────────────────────────────────────────────────
// Inventory Interface
// ─────────────────────────────────────────────────────────────
public:
	/** 인벤토리를 관리하는 컴포넌트를 반환합니다. (아이템 줍기/보관 시 사용) */
	virtual UECInventoryManagerComponent* GetInventoryManagerComponent() const override;


// ─────────────────────────────────────────────────────────────
// Inventory Configuration
// ─────────────────────────────────────────────────────────────
public:
	/** 인벤토리 위젯 정보입니다. */
	UPROPERTY(EditDefaultsOnly)
	FUIConfigWidget InventoryWidget;

	/** 인벤토리 위젯 토글 입력을 처리할 태그입니다. */
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InventoryUITag;

protected:
	/** 게임 시작 시 인벤토리에 기본으로 지급할 아이템 목록 */
	UPROPERTY(EditDefaultsOnly, Category = "변수|인벤토리")
	TArray<FInitialInventoryItem> InitialInventoryItems;
	
private:
	/** 게임 시작 시 InitialInventoryItems를 인벤토리에 등록 */
	void AddInitialInventory();

	/** 교환소를 바라보는 플레이어가 엔딩 제출 아이템을 보유했는지 확인합니다. */
	bool HasEndingSubmissionItem(const FInteractionQuery& InteractQuery) const;

	/** 현재 상호작용을 진행 중인 로컬 플레이어입니다. */
	TWeakObjectPtr<APlayerController> ActivePlayerController;

	UUIManagerSubsystem* GetActiveUIManager() const;
};
