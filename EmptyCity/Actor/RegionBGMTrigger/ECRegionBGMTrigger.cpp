#include "Actor/RegionBGMTrigger/ECRegionBGMTrigger.h"

#include "Components/BoxComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/ECSoundSubsystem.h"

AECRegionBGMTrigger::AECRegionBGMTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetBoxExtent(FVector(300.0, 300.0, 150.0));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleTriggerBeginOverlap);
}

void AECRegionBGMTrigger::PlayConfiguredBGM()
{
	if (!IsValid(BGMRow.DataTable) || BGMRow.RowName.IsNone())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[%s] 재생할 DT_Sound 행이 설정되지 않았습니다."),
			*GetName());
		return;
	}

	const FSoundTableRow* SoundRow = BGMRow.GetRow<FSoundTableRow>(TEXT("AECRegionBGMTrigger::PlayConfiguredBGM"));
	if (!SoundRow || !IsValid(SoundRow->Sound))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[%s] DT_Sound의 [%s] 행에서 유효한 사운드를 찾을 수 없습니다."),
			*GetName(),
			*BGMRow.RowName.ToString());
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UECSoundSubsystem* SoundSubsystem = IsValid(GameInstance)
		? GameInstance->GetSubsystem<UECSoundSubsystem>()
		: nullptr;
	if (!IsValid(SoundSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] SoundSubsystem을 찾을 수 없습니다."), *GetName());
		return;
	}

	switch (TransitionType)
	{
	case EECBGMTransitionType::Immediate:
		SoundSubsystem->PlayBGM(SoundRow->Sound);
		break;

	case EECBGMTransitionType::FadeIn:
		SoundSubsystem->PlayBGMWithFade(SoundRow->Sound, TransitionDuration);
		break;

	case EECBGMTransitionType::CrossFade:
	default:
		SoundSubsystem->CrossFadeBGM(SoundRow->Sound, TransitionDuration);
		break;
	}

	bHasTriggered = true;
}

void AECRegionBGMTrigger::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bTriggerOnce && bHasTriggered)
	{
		return;
	}

	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!IsValid(PlayerController) || OtherActor != PlayerController->GetPawn())
	{
		return;
	}

	PlayConfiguredBGM();
}
