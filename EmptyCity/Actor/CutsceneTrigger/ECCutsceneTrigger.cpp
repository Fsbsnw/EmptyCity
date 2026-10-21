#include "Actor/CutsceneTrigger/ECCutsceneTrigger.h"

#include "Components/BoxComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/ECCutsceneSubsystem.h"

AECCutsceneTrigger::AECCutsceneTrigger()
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
}

void AECCutsceneTrigger::SetCutsceneContextActor(AActor* InContextActor)
{
	CutsceneContextActor = InContextActor;
}

void AECCutsceneTrigger::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!CutsceneTag.IsValid())
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!IsValid(PlayerController) || OtherActor != PlayerController->GetPawn())
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	UECCutsceneSubsystem* CutsceneSubsystem = LocalPlayer
		? LocalPlayer->GetSubsystem<UECCutsceneSubsystem>()
		: nullptr;
	if (IsValid(CutsceneSubsystem))
	{
		CutsceneSubsystem->PlayCutsceneByTag(CutsceneTag, CutsceneContextActor);
	}
}
