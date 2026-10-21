#include "ECRegionTeleportPoint.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"

AECRegionTeleportPoint::AECRegionTeleportPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
	DirectionArrow->SetupAttachment(SceneRoot);
}