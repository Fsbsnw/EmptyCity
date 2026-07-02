// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Bed/ECBed.h"

#include "ECGameplayTags.h"
#include "Character/Player/ECPlayer.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "Components/BoxComponent.h"
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

void AECBed::ShowBedMenu()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    UUIManagerSubsystem* UIManager = PC->GetLocalPlayer()->GetSubsystem<UUIManagerSubsystem>();
    if (!UIManager) return;

    UIManager->OpenDynamicWidget(ECGameplayTags::InputTag_UI_Bed, BedMenuWidgetConfig, this);
}

void AECBed::HideBedMenu(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || !OtherActor->IsA(AECPlayer::StaticClass())) return;
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    UUIManagerSubsystem* UIManager = PC->GetLocalPlayer()->GetSubsystem<UUIManagerSubsystem>();
    if (!UIManager) return;

    UIManager->CloseDynamicWidget(ECGameplayTags::InputTag_UI_Bed);
}

void AECBed::StartSleepSequence()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetLocalPlayer()) return;

    UUIManagerSubsystem* UIManager = PC->GetLocalPlayer()->GetSubsystem<UUIManagerSubsystem>();
    if (!UIManager) return;

    // 침대 메뉴를 닫습니다.
    UIManager->CloseDynamicWidget(ECGameplayTags::InputTag_UI_Bed);

    // 캐릭터, UI 입력 비활성화
    if (AECPlayerController* ECPC = Cast<AECPlayerController>(PC))
    {
        ECPC->SetCinematicInputLocked(true);
    }

    UIManager->PlayFadeOut(FOnFadeEvent::CreateUObject(this, &ThisClass::OnFadeOutCompleted), 0.5f);
}

void AECBed::OnFadeOutCompleted()
{
    // 꿈을 꿉니다.
    if (FMath::RandRange(0.0f, 100.0f) <= DreamChance)
    {
        CurrentSleepPhase = ESleepPhase::Dreaming;
        ShowDreamUI();
    }
    // 꿈을 꾸지 않았습니다. 바로 숙면 메시지로 직행합니다.
    else
    {
        CurrentSleepPhase = ESleepPhase::SleepMessage;
        ShowSleepUI();
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        // 침대 액터가 플레이어의 입력을 가로챕니다. 
        EnableInput(PC);

        // 키 바인딩을 초기화합니다.
        InputComponent->KeyBindings.Empty();
        
        // 플레이어의 입력이 감지되면 깨도록 바인딩
        InputComponent->BindKey(EKeys::AnyKey, IE_Pressed, this, &ThisClass::OnWakeUpInputReceived);
    }
}

void AECBed::OnWakeUpInputReceived()
{
    // 꿈을 꾸는 상태에서 키를 눌렀다면 -> 꿈을 끄고 숙면 메시지를 띄움
    if (CurrentSleepPhase == ESleepPhase::Dreaming)
    {
        HideDreamUI();
        ShowSleepUI();
        CurrentSleepPhase = ESleepPhase::SleepMessage;
    }
    // 숙면 메시지를 보는 상태에서 키를 눌렀다면 -> 진짜로 기상
    else if (CurrentSleepPhase == ESleepPhase::SleepMessage)
    {
        HideSleepUI();
        CurrentSleepPhase = ESleepPhase::None;
        
        EndSleepSequence();
    }
}

void AECBed::EndSleepSequence()
{
    // 키보드 입력이 감지되면 더이상 침대와 관련된 입력을 받지 않습니다.
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        DisableInput(PC);
    }
    UUIManagerSubsystem* UIManager = PC->GetLocalPlayer()->GetSubsystem<UUIManagerSubsystem>();
    if (!UIManager) return;
    
    UIManager->PlayFadeIn(FOnFadeEvent::CreateUObject(this, &ThisClass::OnFadeInCompleted), 0.5f);
}

void AECBed::OnFadeInCompleted()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    // 캐릭터, UI 입력을 활성화합니다.
    if (AECPlayerController* ECPC = Cast<AECPlayerController>(PC))
    {
        ECPC->SetCinematicInputLocked(false);
    }
}

void AECBed::ShowDreamUI()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    UUIManagerSubsystem* UIManager = PC->GetLocalPlayer()->GetSubsystem<UUIManagerSubsystem>();
    if (!UIManager) return;

    UIManager->OpenDynamicWidget(ECGameplayTags::InputTag_UI_Dream, DreamWidgetConfig);
}

void AECBed::HideDreamUI()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    UUIManagerSubsystem* UIManager = PC->GetLocalPlayer()->GetSubsystem<UUIManagerSubsystem>();
    if (!UIManager) return;

    UIManager->CloseDynamicWidget(ECGameplayTags::InputTag_UI_Dream);
}

void AECBed::ShowSleepUI()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    UUIManagerSubsystem* UIManager = PC->GetLocalPlayer()->GetSubsystem<UUIManagerSubsystem>();
    if (!UIManager) return;

    UIManager->OpenDynamicWidget(ECGameplayTags::InputTag_UI_Sleep, SleepWidgetConfig);
}

void AECBed::HideSleepUI()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    UUIManagerSubsystem* UIManager = PC->GetLocalPlayer()->GetSubsystem<UUIManagerSubsystem>();
    if (!UIManager) return;

    UIManager->CloseDynamicWidget(ECGameplayTags::InputTag_UI_Sleep);
}