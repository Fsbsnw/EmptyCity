#include "ECRegionTravelTrigger.h"

#include "ECGameplayTags.h"
#include "Components/BoxComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/ECRegionSubsystem.h"
#include "Subsystem/ECTimeSubsystem.h"

AECRegionTravelTrigger::AECRegionTravelTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetBoxExtent(FVector(100.0, 100.0, 120.0));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleTriggerBeginOverlap);

	TargetRegionTag = ECGameplayTags::MapNode_Location_CultistBase_BossBattle;
	RequiredOriginRegionTag = ECGameplayTags::MapNode_Location_CultistBase;
	MinimumDay = UECRegionSubsystem::CultistBossBattleUnlockDay;
}

bool AECRegionTravelTrigger::IsTravelAvailable() const
{
	if (!TargetRegionTag.IsValid())
	{
		return false;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return false;
	}

	const UECTimeSubsystem* TimeSubsystem = GameInstance->GetSubsystem<UECTimeSubsystem>();
	const UECRegionSubsystem* RegionSubsystem = GameInstance->GetSubsystem<UECRegionSubsystem>();
	if (!IsValid(TimeSubsystem) || !IsValid(RegionSubsystem))
	{
		return false;
	}

	if (TimeSubsystem->GetDay() < MinimumDay)
	{
		return false;
	}

	if (RequiredOriginRegionTag.IsValid()
		&& !RegionSubsystem->GetCurrentRegionTag().MatchesTag(RequiredOriginRegionTag))
	{
		return false;
	}

	return RegionSubsystem->CanEnterRegion(TargetRegionTag);
}

void AECRegionTravelTrigger::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!IsValid(PlayerController) || OtherActor != PlayerController->GetPawn() || !IsTravelAvailable())
	{
		return;
	}

	UECRegionSubsystem::Get(this).RequestTravel(TargetRegionTag);
}
