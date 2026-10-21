#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ECRegionTeleportPoint.generated.h"

class USceneComponent;
class UArrowComponent;

UCLASS()
class EMPTYCITY_API AECRegionTeleportPoint : public AActor
{
	GENERATED_BODY()

public:
	AECRegionTeleportPoint();

	/** 이 이동 지점에 해당하는 지역 태그입니다. */
	FGameplayTag GetRegionTag() const { return RegionTag; }

private:
	/** 이동 지점의 루트 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, Category = "Region")
	TObjectPtr<USceneComponent> SceneRoot;

	/** 에디터에서 이동 후 바라볼 방향을 표시합니다. */
	UPROPERTY(VisibleAnywhere, Category = "Region")
	TObjectPtr<UArrowComponent> DirectionArrow;

	/** 이 이동 지점이 나타내는 지역입니다. */
	UPROPERTY(EditAnywhere, Category = "Region")
	FGameplayTag RegionTag;
};