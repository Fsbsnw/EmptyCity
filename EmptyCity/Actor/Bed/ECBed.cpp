// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Bed/ECBed.h"

#include "ECGameplayTags.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "Components/BoxComponent.h"
#include "Subsystem/ECTimeSubsystem.h"
#include "UI/Subsystem/UIManagerSubsystem.h"

void AECBed::GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& OptionBuilder)
{
    OptionBuilder.AddInteractionOption(Option);
}

AECBed::AECBed()
{
	PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
    SetRootComponent(Mesh);
    
    BoxCollision = CreateDefaultSubobject<UBoxComponent>("Box Collision");
    BoxCollision->SetupAttachment(Mesh);
}

void AECBed::BeginPlay()
{
    Super::BeginPlay();

    BoxCollision->OnComponentEndOverlap.AddDynamic(this, &ThisClass::HideBedMenu);
}

UUIManagerSubsystem* AECBed::GetActiveUIManager() const
{
    return UUIManagerSubsystem::Get(ActivePlayerController.Get());
}

void AECBed::ShowBedMenu(APlayerController* PC)
{
	if (bSleepSequenceInProgress)
	{
		return;
	}

    if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(PC))
    {
        ActivePlayerController = PC;
        
        UIManager->OpenDynamicWidget(ECGameplayTags::InputTag_UI_Bed, BedMenuWidgetConfig, this);
    }
}

void AECBed::CloseBedMenu()
{
	if (bSleepSequenceInProgress)
	{
		return;
	}

	if (UUIManagerSubsystem* UIManager = GetActiveUIManager())
	{
		UIManager->CloseDynamicWidget(ECGameplayTags::InputTag_UI_Bed);
	}

	ActivePlayerController.Reset();
}

void AECBed::HideBedMenu(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bSleepSequenceInProgress)
	{
		return;
	}

    APawn* PlayerPawn = Cast<APawn>(OtherActor);
    if (PlayerPawn == nullptr)
    {
        return;
    }

    APlayerController* LeavingPC = Cast<APlayerController>(PlayerPawn->GetController());

    if (LeavingPC == nullptr || LeavingPC != ActivePlayerController.Get())
    {
        return;
    }

	CloseBedMenu();
}

void AECBed::StartSleepSequence()
{
    UUIManagerSubsystem* UIManager = GetActiveUIManager();
    if (UIManager == nullptr || bSleepSequenceInProgress)
    {
        return;
    }

    // 침대 메뉴를 닫습니다.
    UIManager->CloseDynamicWidget(ECGameplayTags::InputTag_UI_Bed);

    // 캐릭터, UI 입력 비활성화
    if (AECPlayerController* ECPC = Cast<AECPlayerController>(ActivePlayerController.Get()))
    {
        ECPC->SetPlayerInputLocked(true);
    }

	bSleepSequenceInProgress = true;
	const bool bFadeStarted = UIManager->PlayFadeTransition(
		FOnFadeEvent::CreateUObject(this, &ThisClass::OnFadeOutCompleted),
		SleepFadeSpeed,
		FOnFadeEvent::CreateUObject(this, &ThisClass::OnFadeInCompleted));

	if (!bFadeStarted)
	{
		bSleepSequenceInProgress = false;
		if (AECPlayerController* ECPC = Cast<AECPlayerController>(ActivePlayerController.Get()))
		{
			ECPC->SetPlayerInputLocked(false);
		}
	}
}

void AECBed::OnFadeOutCompleted()
{
	UECTimeSubsystem::Get(this).SleepUntilTomorrow();
}

void AECBed::OnFadeInCompleted()
{
    // 캐릭터, UI 입력을 활성화합니다.
    if (AECPlayerController* PC = Cast<AECPlayerController>(ActivePlayerController.Get()))
    {
        PC->SetPlayerInputLocked(false);
    }

	bSleepSequenceInProgress = false;
	ActivePlayerController.Reset();
}
