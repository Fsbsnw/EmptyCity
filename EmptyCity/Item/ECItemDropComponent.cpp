#include "ECItemDropComponent.h"
#include "Actor/Farming/ECFarmingResourceActor.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Data/Item/ECItemDropTableRow.h"
#include "Data/Item/ECItemTableRow.h"
#include "ECPickupableItem.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "Inventory/ECInventoryManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/ECItemDataSubsystem.h"

namespace
{
const FName CoinItemId(TEXT("Coin"));
constexpr float MinimumGroundNormalZ = 0.6f;
constexpr int32 MaxGroundTraceIterations = 8;

UECInventoryManagerComponent* FindInventoryOnActor(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	if (UECInventoryManagerComponent* Inventory =
		Actor->FindComponentByClass<UECInventoryManagerComponent>())
	{
		return Inventory;
	}

	const APawn* Pawn = Cast<APawn>(Actor);
	return Pawn && Pawn->GetController()
		? Pawn->GetController()->FindComponentByClass<UECInventoryManagerComponent>()
		: nullptr;
}

UECInventoryManagerComponent* ResolveRecipientInventory(
	const UObject* WorldContextObject,
	AActor* PreferredRecipient,
	AActor* DropOwner)
{
	if (UECInventoryManagerComponent* Inventory = FindInventoryOnActor(PreferredRecipient))
	{
		return Inventory;
	}

	if (IsValid(PreferredRecipient))
	{
		if (UECInventoryManagerComponent* Inventory = FindInventoryOnActor(PreferredRecipient->GetOwner()))
		{
			return Inventory;
		}

		if (UECInventoryManagerComponent* Inventory =
			FindInventoryOnActor(PreferredRecipient->GetInstigatorController()))
		{
			return Inventory;
		}
	}

	if (IsValid(DropOwner))
	{
		if (UECInventoryManagerComponent* Inventory =
			FindInventoryOnActor(DropOwner->GetInstigatorController()))
		{
			return Inventory;
		}
	}

	return FindInventoryOnActor(UGameplayStatics::GetPlayerController(WorldContextObject, 0));
}

AECPlayerState* FindPlayerStateOnActor(AActor* Actor)
{
	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		return Pawn->GetPlayerState<AECPlayerState>();
	}

	if (const AController* Controller = Cast<AController>(Actor))
	{
		return Controller->GetPlayerState<AECPlayerState>();
	}

	return nullptr;
}

AECPlayerState* ResolveRecipientPlayerState(
	const UObject* WorldContextObject,
	AActor* PreferredRecipient,
	AActor* DropOwner)
{
	if (AECPlayerState* PlayerState = FindPlayerStateOnActor(PreferredRecipient))
	{
		return PlayerState;
	}

	if (IsValid(PreferredRecipient))
	{
		if (AECPlayerState* PlayerState = FindPlayerStateOnActor(PreferredRecipient->GetOwner()))
		{
			return PlayerState;
		}

		if (AECPlayerState* PlayerState =
			FindPlayerStateOnActor(PreferredRecipient->GetInstigatorController()))
		{
			return PlayerState;
		}
	}

	if (IsValid(DropOwner))
	{
		if (AECPlayerState* PlayerState =
			FindPlayerStateOnActor(DropOwner->GetInstigatorController()))
		{
			return PlayerState;
		}
	}

	return FindPlayerStateOnActor(UGameplayStatics::GetPlayerController(WorldContextObject, 0));
}
}

DEFINE_LOG_CATEGORY_STATIC(LogECItemDrop, Log, All);

UECItemDropComponent::UECItemDropComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FInventoryPickup UECItemDropComponent::GenerateDrop() const
{
	FInventoryPickup Pickup;
	const FECItemDropTableRow* DropData =
		DropTableRow.GetRow<FECItemDropTableRow>(TEXT("UECItemDropComponent::GenerateDrop"));
	if (!DropData)
	{
		UE_LOG(
			LogECItemDrop,
			Warning,
			TEXT("[%s] 유효한 아이템 드롭 테이블 행이 설정되지 않았습니다."),
			*GetNameSafe(GetOwner()));
		return Pickup;
	}

	// 각 드롭 항목의 확률과 수량을 독립적으로 계산합니다.
	for (const FECItemDropEntry& DropEntry : DropData->DropEntries)
	{
		if (FMath::RandRange(1, 100) > FMath::Clamp(DropEntry.DropChance, 0, 100))
		{
			continue;
		}

		const FECItemTableRow* ItemData =
			DropEntry.Item.GetRow<FECItemTableRow>(TEXT("UECItemDropComponent::GenerateDrop"));
		if (!ItemData)
		{
			UE_LOG(
				LogECItemDrop,
				Warning,
				TEXT("[%s] 드롭 항목이 유효한 DT_Item 행을 참조하지 않습니다: %s"),
				*GetNameSafe(GetOwner()),
				*DropEntry.Item.RowName.ToString());
			continue;
		}

		TSubclassOf<UECInventoryItemDefinition> ItemDefinition =
			ItemData->ItemDefinition.LoadSynchronous();
		if (!ItemDefinition)
		{
			UE_LOG(
				LogECItemDrop,
				Warning,
				TEXT("[%s] DT_Item 행의 Item Definition을 로드하지 못했습니다: %s"),
				*GetNameSafe(GetOwner()),
				*DropEntry.Item.RowName.ToString());
			continue;
		}

		const int32 MinimumCount = FMath::Max(1, FMath::Min(DropEntry.MinCount, DropEntry.MaxCount));
		const int32 MaximumCount = FMath::Max(1, FMath::Max(DropEntry.MinCount, DropEntry.MaxCount));
		FPickupTemplate& PickupTemplate = Pickup.Templates.AddDefaulted_GetRef();
		PickupTemplate.ItemDef = ItemDefinition;
		PickupTemplate.StackCount = FMath::RandRange(MinimumCount, MaximumCount);
	}
	return Pickup;
}

void UECItemDropComponent::SpawnDrop(AActor* RecipientActor) const
{
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (!World || World->bIsTearingDown || !IsValid(OwnerActor) || !OwnerActor->HasAuthority())
	{
		return;
	}

	// 컴포넌트에 설정된 드롭 행으로 생성할 아이템 목록을 계산합니다.
	const FInventoryPickup DropResult = GenerateDrop();
	if (DropResult.Templates.IsEmpty())
	{
		return;
	}

	UECInventoryManagerComponent* RecipientInventory =
		ResolveRecipientInventory(this, RecipientActor, OwnerActor);
	AECPlayerState* RecipientPlayerState =
		ResolveRecipientPlayerState(this, RecipientActor, OwnerActor);
	UECItemDataSubsystem& ItemDataSubsystem = UECItemDataSubsystem::Get(this);
	TArray<FPickupTemplate> WorldDropTemplates;
	WorldDropTemplates.Reserve(DropResult.Templates.Num());

	for (const FPickupTemplate& PickupTemplate : DropResult.Templates)
	{
		const FName ItemId = ItemDataSubsystem.FindItemId(PickupTemplate.ItemDef);
		if (ItemId == CoinItemId)
		{
			if (IsValid(RecipientPlayerState))
			{
				RecipientPlayerState->AddCoin(PickupTemplate.StackCount);
			}
			else
			{
				UE_LOG(
					LogECItemDrop,
					Warning,
					TEXT("[%s] Coin 드롭을 받을 PlayerState를 찾지 못했습니다: %d"),
					*GetNameSafe(OwnerActor),
					PickupTemplate.StackCount);
			}
			continue;
		}

		const FECItemTableRow* ItemData =
			ItemDataSubsystem.FindItemData(ItemId);
		UStaticMesh* WorldMesh = ItemData && !ItemData->WorldMesh.IsNull()
			? ItemData->WorldMesh.LoadSynchronous()
			: nullptr;

		if (IsValid(WorldMesh))
		{
			WorldDropTemplates.Add(PickupTemplate);
			continue;
		}

		if (!RecipientInventory
			|| !RecipientInventory->AddItemDefinition(
				PickupTemplate.ItemDef,
				PickupTemplate.StackCount))
		{
			UE_LOG(
				LogECItemDrop,
				Warning,
				TEXT("[%s] 월드 메시가 없는 아이템을 인벤토리에 추가하지 못했습니다: %s x%d"),
				*GetNameSafe(OwnerActor),
				*GetNameSafe(PickupTemplate.ItemDef),
				PickupTemplate.StackCount);
		}
	}

	if (WorldDropTemplates.IsEmpty())
	{
		return;
	}

	FCollisionQueryParams GroundQueryParams(SCENE_QUERY_STAT(ItemDropGround), false);
	GroundQueryParams.AddIgnoredActor(OwnerActor);
	const FRotator DropRotation(0.0f, OwnerActor->GetActorRotation().Yaw, 0.0f);
	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const FVector GroundTraceStart(
		OwnerLocation.X,
		OwnerLocation.Y,
		OwnerLocation.Z + FMath::Max(GroundTraceUpDistance, 1.0f));
	const FVector GroundTraceEnd(
		OwnerLocation.X,
		OwnerLocation.Y,
		OwnerLocation.Z - FMath::Max(GroundTraceDownDistance, 0.0f));

	FHitResult SharedGroundHit;
	const FCollisionObjectQueryParams GroundObjectQueryParams(ECC_WorldStatic);
	bool bFoundSharedGround = false;

	// WorldStatic에는 바닥뿐 아니라 벽도 포함됩니다. 수직 트레이스에 먼저 걸린 벽 콜리전은
	// 무시 목록에 추가하고, 위쪽을 향한 면을 찾을 때까지 아래 지면을 계속 검사합니다.
	for (int32 TraceIndex = 0; TraceIndex < MaxGroundTraceIterations; ++TraceIndex)
	{
		FHitResult GroundCandidate;
		if (!World->LineTraceSingleByObjectType(
			GroundCandidate,
			GroundTraceStart,
			GroundTraceEnd,
			GroundObjectQueryParams,
			GroundQueryParams))
		{
			break;
		}

		AActor* HitActor = GroundCandidate.GetActor();
		const bool bHitFarmingResource = IsValid(Cast<AECFarmingResourceActor>(HitActor));
		if (!bHitFarmingResource
			&& !GroundCandidate.bStartPenetrating
			&& GroundCandidate.ImpactNormal.Z >= MinimumGroundNormalZ)
		{
			SharedGroundHit = GroundCandidate;
			bFoundSharedGround = true;
			break;
		}

		if (bHitFarmingResource)
		{
			GroundQueryParams.AddIgnoredActor(HitActor);
		}
		else if (UPrimitiveComponent* HitComponent = GroundCandidate.GetComponent())
		{
			GroundQueryParams.AddIgnoredComponent(HitComponent);
		}
		else if (HitActor)
		{
			GroundQueryParams.AddIgnoredActor(HitActor);
		}
		else
		{
			break;
		}
	}

	const float SharedGroundZ = bFoundSharedGround
		? SharedGroundHit.ImpactPoint.Z
		: OwnerLocation.Z;

	if (!bFoundSharedGround)
	{
		UE_LOG(
			LogECItemDrop,
			Warning,
			TEXT("[%s] 공통 드랍 지면을 찾지 못해 액터의 월드 Z를 사용합니다."),
			*GetNameSafe(OwnerActor));
	}

	const FVector2D DropCenter(OwnerLocation.X, OwnerLocation.Y);
	const float MinimumSpacing = FMath::Max(MinimumDropSpacing, 0.0f);
	const int32 WorldDropCount = WorldDropTemplates.Num();
	const float RequiredRadius = WorldDropCount > 1 && MinimumSpacing > 0.0f
		? MinimumSpacing / (2.0f * FMath::Sin(PI / static_cast<float>(WorldDropCount)))
		: 0.0f;
	const float LayoutRadius = FMath::Max(FMath::Max(DropSpawnRadius, 0.0f), RequiredRadius);
	const float StartAngle = FMath::FRandRange(0.0f, UE_TWO_PI);

	// 드롭 원점 바로 아래의 정적 지면 높이를 공통으로 사용하고 아이템을 원형으로 분산합니다.
	// 원의 반지름은 인접한 아이템 중심 간 거리가 MinimumDropSpacing 이상이 되도록 자동 확장합니다.
	for (int32 DropIndex = 0; DropIndex < WorldDropTemplates.Num(); ++DropIndex)
	{
		const FPickupTemplate& WorldDropTemplate = WorldDropTemplates[DropIndex];
		const FECItemTableRow* ItemData =
			ItemDataSubsystem.FindItemData(WorldDropTemplate.ItemDef);
		UStaticMesh* WorldMesh = ItemData && !ItemData->WorldMesh.IsNull()
			? ItemData->WorldMesh.LoadSynchronous()
			: nullptr;
		if (!ItemData || !IsValid(WorldMesh))
		{
			UE_LOG(
				LogECItemDrop,
				Warning,
				TEXT("[%s] 월드 드랍 직전에 아이템 메시 데이터를 찾지 못했습니다: %s"),
				*GetNameSafe(OwnerActor),
				*GetNameSafe(WorldDropTemplate.ItemDef));
			continue;
		}

		// 메시 피벗과 WorldScale을 반영한 액터 원점 기준 월드 바운드입니다.
		const FBoxSphereBounds RelativeMeshBounds = WorldMesh->GetBounds().TransformBy(
			FTransform(DropRotation, FVector::ZeroVector, ItemData->WorldScale));
		const float MeshBottomOffset =
			RelativeMeshBounds.Origin.Z - RelativeMeshBounds.BoxExtent.Z;
		const float SpawnOriginZ =
			SharedGroundZ - MeshBottomOffset + FMath::Max(GroundClearance, 0.0f);
		const float DropAngle = StartAngle
			+ UE_TWO_PI * static_cast<float>(DropIndex) / static_cast<float>(WorldDropCount);
		const FVector2D SpawnPoint = DropCenter + FVector2D(
			FMath::Cos(DropAngle) * LayoutRadius,
			FMath::Sin(DropAngle) * LayoutRadius);
		const FVector SpawnLocation(
			SpawnPoint.X,
			SpawnPoint.Y,
			SpawnOriginZ);
		const FTransform DropTransform(DropRotation, SpawnLocation);

		// 드롭 액터는 곧 제거될 적의 소유/스트리밍 레벨에 종속되지 않도록
		// Persistent Level에 독립 액터로 생성합니다.
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = nullptr;
		SpawnParameters.Instigator = OwnerActor->GetInstigator();
		SpawnParameters.OverrideLevel = World->PersistentLevel;
		SpawnParameters.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.bDeferConstruction = true;
		AECPickupableItem* PickupActor = World->SpawnActor<AECPickupableItem>(
			AECPickupableItem::StaticClass(),
			DropTransform,
			SpawnParameters);
		if (!PickupActor)
		{
			UE_LOG(LogECItemDrop, Warning, TEXT("[%s] 월드 픽업 액터 생성에 실패했습니다."), *GetNameSafe(OwnerActor));
			continue;
		}

		FInventoryPickup PickupInventory;
		PickupInventory.Templates.Add(WorldDropTemplate);
		PickupActor->SetPickupInventory(PickupInventory);
		PickupActor->FinishSpawning(DropTransform);
		UE_LOG(
			LogECItemDrop,
			Log,
			TEXT("[%s] 월드 드랍 생성: %s x%d, 위치=%s"),
			*GetNameSafe(OwnerActor),
			*GetNameSafe(WorldDropTemplate.ItemDef),
			WorldDropTemplate.StackCount,
			*SpawnLocation.ToCompactString());
	}
}
