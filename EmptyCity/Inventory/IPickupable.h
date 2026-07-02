// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "UObject/Interface.h"
#include "UObject/ObjectPtr.h"
#include "IPickupable.generated.h"

template <typename InterfaceType> class TScriptInterface;

class AActor;
class UECInventoryItemDefinition;
class UECInventoryItemInstance;
class UECInventoryManagerComponent;
class UObject;
struct FFrame;

/**
 * 아직 월드에 인스턴스화되지 않은 "아이템 설계도 + 개수" 묶음입니다.
 * 줍는 순간 인벤토리에서 이 정보로 새 아이템 인스턴스를 생성합니다. (예: "포션 설계도 3개")
 */
USTRUCT(BlueprintType)
struct FPickupTemplate
{
	GENERATED_BODY()

public:
	/** 한 번에 주워 들일 개수입니다. */
	UPROPERTY(EditAnywhere)
	int32 StackCount = 1;

	/** 생성할 아이템의 설계도입니다. */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UECInventoryItemDefinition> ItemDef;
};

/**
 * 이미 생성되어 런타임 상태(내구도 등)를 가진 아이템 인스턴스 하나를 담는 묶음입니다.
 * 설계도로부터 새로 만드는 Template과 달리, 기존 인스턴스를 그대로 옮길 때 사용합니다.
 */
USTRUCT(BlueprintType)
struct FPickupInstance
{
	GENERATED_BODY()

public:
	/** 그대로 인벤토리로 옮길 아이템 인스턴스입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UECInventoryItemInstance> Item = nullptr;
};

/**
 * 줍기 대상이 인벤토리에 넘겨줄 "내용물" 전체입니다.
 * 설계도로 새로 만들 것(Templates)과 기존 인스턴스 그대로 옮길 것(Instances)을 함께 담아,
 * 한 번 주울 때 여러 아이템을 동시에 처리할 수 있습니다.
 */
USTRUCT(BlueprintType)
struct FInventoryPickup
{
	GENERATED_BODY()

public:
	/** 설계도로부터 새로 생성할 아이템 목록입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FPickupTemplate> Templates;
};

UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UPickupable : public UInterface
{
	GENERATED_BODY()
};

/**
 * "주울 수 있는 대상"이 구현하는 인터페이스입니다.
 * 떨어진 아이템 액터나 그 컴포넌트가 이 인터페이스를 구현해, 자신이 어떤 내용물을 주는지 알려줍니다.
 */
class IPickupable
{
	GENERATED_BODY()

public:
	/** 이 대상이 인벤토리에 넘겨줄 내용물(설계도/인스턴스 묶음)을 반환합니다. */
	UFUNCTION(BlueprintCallable)
	virtual FInventoryPickup GetPickupInventory() const = 0;
};

/**
 * 줍기 처리에 필요한 보조 함수들을 모아 둔 정적 함수 라이브러리입니다.
 * "액터에서 Pickupable 대상 찾기"와 "Pickupable의 내용물을 인벤토리에 추가하기"를 담당합니다.
 */
UCLASS()
class UPickupableStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UPickupableStatics();

public:
	/** 액터 자신 또는 그 컴포넌트 중에서 IPickupable을 구현한 첫 번째 대상을 찾아 반환합니다. (없으면 빈 인터페이스) */
	UFUNCTION(BlueprintPure)
	static TScriptInterface<IPickupable> GetFirstPickupableFromActor(AActor* Actor);

	/** Pickup 대상의 내용물을 인벤토리 컴포넌트에 추가합니다. 서버 권한에서만 동작합니다. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (WorldContext = "Ability"))
	static void AddPickupToInventory(UECInventoryManagerComponent* InventoryComponent, TScriptInterface<IPickupable> Pickup);
};
