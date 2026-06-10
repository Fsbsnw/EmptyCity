#include "ECSoundSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(ALogSoundSubsystem);

namespace SoundSubsystemConstants
{
    // 페이드 틱 간격입니다.
    constexpr float FadeTickInterval = 0.016f;

    // 볼륨 최솟값입니다.
    constexpr float MinVolume = 0.0f;

    // 볼륨 최댓값입니다.
    constexpr float MaxVolume = 1.0f;

    // 볼륨이 완전히 꺼진 것으로 판단하는 임계값입니다.
    constexpr float SilenceThreshold = 0.01f;
}


void UECSoundSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Config에서 지정한 경로로 사운드 데이터 테이블을 로드합니다.
    if (!SoundDataTablePath.IsNull())
    {
        SoundDataTable = Cast<UDataTable>(SoundDataTablePath.TryLoad());
        UE_LOG(ALogSoundSubsystem, Warning, TEXT("USoundSubsystem::Initialize : 사운드 데이터 테이블을 로드했습니다"));
    }

    // SoundMix를 한 번만 Push합니다.
    if (IsValid(MasterSoundMix))
    {
        UGameplayStatics::PushSoundMixModifier(this, MasterSoundMix);
        UE_LOG(ALogSoundSubsystem, Warning, TEXT("USoundSubsystem::Initialize : MasterSoundMix를 설정했습니다"));
    }
    
    // 새 월드(레벨 재시작 포함)의 액터 초기화가 끝날 때마다 사운드 풀을 재생성하도록 바인딩합니다.
    WorldInitializedHandle = FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &ThisClass::HandleWorldInitializedActors);
}


void UECSoundSubsystem::Deinitialize()
{
    FWorldDelegates::OnWorldInitializedActors.Remove(WorldInitializedHandle);

    // 페이드 타이머를 제거합니다.
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        World->GetTimerManager().ClearTimer(FadeTimerHandle);
    }

    // BGM 풀의 모든 컴포넌트를 정리합니다.
    for (UAudioComponent* Comp : BGMPool)
    {
        if (IsValid(Comp))
        {
            Comp->Stop();
            Comp->DestroyComponent();
        }
    }
    BGMPool.Empty();

    // 재생 중인 SFX 컴포넌트를 정리합니다.
    for (UAudioComponent* Comp : ActiveSFXPool)
    {
        if (IsValid(Comp))
        {
            Comp->Stop();
            Comp->DestroyComponent();
        }
    }
    ActiveSFXPool.Empty();

    // 대기 중인 SFX 컴포넌트를 정리합니다.
    for (UAudioComponent* Comp : AvailableSFXPool)
    {
        if (IsValid(Comp))
        {
            Comp->DestroyComponent();
        }
    }
    AvailableSFXPool.Empty();

    // Sound Mix를 해제합니다.
    if (IsValid(MasterSoundMix))
    {
        UGameplayStatics::PopSoundMixModifier(this, MasterSoundMix);
    }

    Super::Deinitialize();
}


bool UECSoundSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer))
    {
        return false;
    }

    // 이 클래스를 상속한 클래스(BP_SoundSubsystem 등)가 존재하면
    // 가장 하위 클래스만 인스턴스로 생성되도록 false를 반환합니다.
    TArray<UClass*> DerivedClasses;
    GetDerivedClasses(GetClass(), DerivedClasses, false);

    return DerivedClasses.Num() == 0;
}


UECSoundSubsystem& UECSoundSubsystem::Get(const UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
    check(World);
    UECSoundSubsystem* Router = UGameInstance::GetSubsystem<UECSoundSubsystem>(World->GetGameInstance());
    check(Router);
    return *Router;
}


void UECSoundSubsystem::PlayBGM(USoundBase* NewBGM)
{
    if (!IsValid(NewBGM))
    {
        return;
    }

    // 진행 중인 페이드가 있다면 타이머를 중단합니다.
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        World->GetTimerManager().ClearTimer(FadeTimerHandle);
    }

    FadeState = EBGMFadeState::None;

    // 기존 BGM을 즉시 중단합니다.
    UAudioComponent* CurrentComp = GetCurrentBGMComponent();
    if (IsValid(CurrentComp) && CurrentComp->IsPlaying())
    {
        CurrentComp->Stop();
    }

    // 새로운 BGM을 즉시 재생합니다.
    UAudioComponent* NextComp = GetAvailableBGMComponent();
    if (IsValid(NextComp))
    {
        NextComp->bAllowSpatialization = false;
        NextComp->SetSound(NewBGM);
        NextComp->SetVolumeMultiplier(1.0f);
        NextComp->Play();
        CurrentBGMIndex = BGMPool.IndexOfByKey(NextComp);
        
        UE_LOG(ALogSoundSubsystem, Warning, TEXT("USoundSubsystem::PlayBGM : [%s]를 재생합니다."), *NewBGM->GetName());
    }
}


void UECSoundSubsystem::PlayBGMWithFade(USoundBase* NewBGM, float FadeInDuration)
{
    if (!IsValid(NewBGM))
    {
        return;
    }
    
    FadeState = EBGMFadeState::None;

    // 기존 BGM을 중단합니다.
    UAudioComponent* CurrentComp = GetCurrentBGMComponent();
    if (IsValid(CurrentComp) && CurrentComp->IsPlaying())
    {
        CurrentComp->Stop();
    }

    // 새로운 BGM을 볼륨 0에서 시작하여 페이드 인합니다.
    UAudioComponent* NextComp = GetAvailableBGMComponent();
    if (!IsValid(NextComp))
    {
        return;
    }

    NextComp->SetSound(NewBGM);
    NextComp->SetVolumeMultiplier(SoundSubsystemConstants::MinVolume);
    NextComp->Play();
    UE_LOG(ALogSoundSubsystem, Warning, TEXT("USoundSubsystem::PlayBGMWithFade : [%s]를 재생합니다."), *NewBGM->GetName());

    CurrentBGMIndex = BGMPool.IndexOfByKey(NextComp);
    FadeState = EBGMFadeState::FadingIn;
    FadeDuration = FMath::Max(FadeInDuration, SoundSubsystemConstants::FadeTickInterval);
    FadeElapsed = 0.0f;

    // 페이드 틱 타이머를 시작합니다.
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        World->GetTimerManager().SetTimer(
            FadeTimerHandle,
            this,
            &ThisClass::TickFade,
            SoundSubsystemConstants::FadeTickInterval,
            true
        );
    }
}


void UECSoundSubsystem::StopBGM()
{
    // 진행 중인 페이드 타이머를 중단합니다.
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        World->GetTimerManager().ClearTimer(FadeTimerHandle);
    }

    FadeState = EBGMFadeState::None;

    // 현재 재생 중인 BGM을 즉시 중단합니다.
    UAudioComponent* CurrentComp = GetCurrentBGMComponent();
    if (IsValid(CurrentComp))
    {
        CurrentComp->Stop();
    }
}


void UECSoundSubsystem::StopBGMWithFade(float FadeOutDuration)
{
    UAudioComponent* CurrentComp = GetCurrentBGMComponent();
    if (!IsValid(CurrentComp) || !CurrentComp->IsPlaying())
    {
        return;
    }

    FadeState = EBGMFadeState::FadingOut;
    FadeDuration = FMath::Max(FadeOutDuration, SoundSubsystemConstants::FadeTickInterval);
    FadeElapsed = 0.0f;

    // 페이드 틱 타이머를 시작합니다.
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        World->GetTimerManager().SetTimer(
            FadeTimerHandle,
            this,
            &ThisClass::TickFade,
            SoundSubsystemConstants::FadeTickInterval,
            true
        );
    }
}


void UECSoundSubsystem::CrossFadeBGM(USoundBase* NewBGM, float CrossFadeDuration)
{
    if (!IsValid(NewBGM))
    {
        return;
    }
    
    // 다음 BGM 슬롯을 현재와 다른 인덱스로 설정합니다.
    NextBGMIndex = (CurrentBGMIndex + 1) % BGMPool.Num();

    UAudioComponent* NextComp = BGMPool[NextBGMIndex];
    if (!IsValid(NextComp))
    {
        return;
    }

    // 새로운 BGM을 볼륨 0에서 시작합니다.
    NextComp->SetSound(NewBGM);
    NextComp->SetVolumeMultiplier(SoundSubsystemConstants::MinVolume);
    NextComp->Play();
    
    UE_LOG(ALogSoundSubsystem, Warning, TEXT("USoundSubsystem::CrossFadeBGM : [%s]를 재생합니다."), *NewBGM->GetName());

    FadeState = EBGMFadeState::CrossFading;
    FadeDuration = FMath::Max(CrossFadeDuration, SoundSubsystemConstants::FadeTickInterval);
    FadeElapsed = 0.0f;

    // 크로스페이드 틱 타이머를 시작합니다.
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        World->GetTimerManager().SetTimer(
            FadeTimerHandle,
            this,
            &ThisClass::TickFade,
            SoundSubsystemConstants::FadeTickInterval,
            true
        );
    }
}


UAudioComponent* UECSoundSubsystem::PlaySFX(USoundBase* SFXSound)
{
    if (!IsValid(SFXSound))
    {
        return nullptr;
    }

    UAudioComponent* SFXComp = GetAvailableSFXComponent();
    if (!IsValid(SFXComp))
    {
        return nullptr;
    }

    // 2D 사운드로 재생합니다.
    SFXComp->bAllowSpatialization = false;
    SFXComp->SetSound(SFXSound);
    SFXComp->SetVolumeMultiplier(1.0f);
    SFXComp->Play();
    
    UE_LOG(ALogSoundSubsystem, Warning, TEXT("USoundSubsystem::PlaySFX : [%s]를 재생합니다."), *SFXSound->GetName());

    return SFXComp;
}


UAudioComponent* UECSoundSubsystem::PlaySFXAtLocation(USoundBase* SFXSound, const FVector& Location)
{
    if (!IsValid(SFXSound))
    {
        return nullptr;
    }

    UAudioComponent* SFXComp = GetAvailableSFXComponent();
    if (!IsValid(SFXComp))
    {
        return nullptr;
    }

    // 3D 위치 기반 사운드로 재생합니다.
    SFXComp->bAllowSpatialization = true;
    SFXComp->SetWorldLocation(Location);
    SFXComp->SetSound(SFXSound);
    SFXComp->SetVolumeMultiplier(1.0f);
    SFXComp->Play();
    
    UE_LOG(ALogSoundSubsystem, Warning, TEXT("USoundSubsystem::PlaySFXAtLocation : [%s]를 재생합니다."), *SFXSound->GetName());

    return SFXComp;
}


void UECSoundSubsystem::StopAllSFX()
{
    for (UAudioComponent* Comp : ActiveSFXPool)
    {
        if (IsValid(Comp))
        {
            Comp->Stop();
        }
    }
    
    // Stop()은 OnAudioFinished를 발생시키지 않으므로 수동으로 풀을 돌려놓습니다.
    AvailableSFXPool.Append(ActiveSFXPool);
    ActiveSFXPool.Empty();
}


void UECSoundSubsystem::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, SoundSubsystemConstants::MinVolume, SoundSubsystemConstants::MaxVolume);
    ApplySoundMixModifier();
}


void UECSoundSubsystem::SetBGMVolume(float Volume)
{
    BGMVolume = FMath::Clamp(Volume, SoundSubsystemConstants::MinVolume, SoundSubsystemConstants::MaxVolume);
    ApplySoundMixModifier();
}


void UECSoundSubsystem::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, SoundSubsystemConstants::MinVolume, SoundSubsystemConstants::MaxVolume);
    ApplySoundMixModifier();
}


void UECSoundSubsystem::TickFade()
{
    // DeltaTime 대신 타이머 고정 간격을 사용합니다.
    FadeElapsed += SoundSubsystemConstants::FadeTickInterval;
    const float Alpha = FMath::Clamp(FadeElapsed / FadeDuration, 0.0f, 1.0f);

    UAudioComponent* CurrentComp = GetCurrentBGMComponent();
    UWorld* World = GetWorld();

    // 컴포넌트 multiplier는 0~1 페이드 alpha만 담당하고, 실제 볼륨은 SoundMix가 처리합니다.
    constexpr float FullVolume = 1.0f;

    switch (FadeState)
    {
    case EBGMFadeState::FadingIn:
    {
        // 볼륨을 0에서 목표 볼륨으로 증가합니다.
        if (IsValid(CurrentComp))
        {
            CurrentComp->SetVolumeMultiplier(FMath::Lerp(0.0f, FullVolume, Alpha));
        }

        if (Alpha >= 1.0f)
        {
            FadeState = EBGMFadeState::None;
            if (IsValid(World))
            {
                World->GetTimerManager().ClearTimer(FadeTimerHandle);
            }
        }
        break;
    }

    case EBGMFadeState::FadingOut:
    {
        // 볼륨을 현재 볼륨에서 0으로 감소합니다.
        if (IsValid(CurrentComp))
        {
            CurrentComp->SetVolumeMultiplier(FMath::Lerp(FullVolume, 0.0f, Alpha));
        }

        if (Alpha >= 1.0f)
        {
            if (IsValid(CurrentComp))
            {
                CurrentComp->Stop();
            }
            FadeState = EBGMFadeState::None;
            if (IsValid(World))
            {
                World->GetTimerManager().ClearTimer(FadeTimerHandle);
            }
        }
        break;
    }

    case EBGMFadeState::CrossFading:
    {
        UAudioComponent* NextComp = GetNextBGMComponent();

        // 현재 BGM은 페이드 아웃, 다음 BGM은 페이드 인합니다.
        if (IsValid(CurrentComp))
        {
            CurrentComp->SetVolumeMultiplier(FMath::Lerp(FullVolume, 0.0f, Alpha));
        }
        if (IsValid(NextComp))
        {
            NextComp->SetVolumeMultiplier(FMath::Lerp(0.0f, FullVolume, Alpha));
        }

        if (Alpha >= 1.0f)
        {
            // 이전 BGM을 중단하고 인덱스를 교체합니다.
            if (IsValid(CurrentComp))
            {
                CurrentComp->Stop();
            }
            CurrentBGMIndex = NextBGMIndex;

            FadeState = EBGMFadeState::None;
            if (IsValid(World))
            {
                World->GetTimerManager().ClearTimer(FadeTimerHandle);
            }
        }
        break;
    }

    default:
        break;
    }
}


UAudioComponent* UECSoundSubsystem::GetAvailableBGMComponent()
{
    // 현재 인덱스와 다른 BGM 슬롯을 반환합니다.
    for (int32 i = 0; i < BGMPool.Num(); ++i)
    {
        if (i != CurrentBGMIndex && IsValid(BGMPool[i]))
        {
            return BGMPool[i];
        }
    }
    return BGMPool.IsValidIndex(CurrentBGMIndex) ? BGMPool[CurrentBGMIndex] : nullptr;
}


UAudioComponent* UECSoundSubsystem::GetCurrentBGMComponent() const
{
    if (BGMPool.IsValidIndex(CurrentBGMIndex))
    {
        return BGMPool[CurrentBGMIndex];
    }
    return nullptr;
}


UAudioComponent* UECSoundSubsystem::GetNextBGMComponent() const
{
    if (BGMPool.IsValidIndex(NextBGMIndex))
    {
        return BGMPool[NextBGMIndex];
    }
    return nullptr;
}


UAudioComponent* UECSoundSubsystem::GetAvailableSFXComponent()
{
    // 가용 풀에 슬롯이 있으면 O(1)으로 꺼냅니다.
    if (AvailableSFXPool.Num() > 0)
    {
        UAudioComponent* Comp = AvailableSFXPool.Pop();
        ActiveSFXPool.Emplace(Comp);
        return Comp;
    }

    // 가용 슬롯이 없으면 새 컴포넌트를 동적으로 생성합니다.
    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return nullptr;
    }

    UAudioComponent* NewComp = NewObject<UAudioComponent>(World);
    if (!IsValid(NewComp))
    {
        return nullptr;
    }

    NewComp->bAutoActivate = false;
    NewComp->bAutoDestroy = false;
    NewComp->RegisterComponentWithWorld(World);

    NewComp->OnAudioFinishedNative.AddWeakLambda(this, [this, NewComp](UAudioComponent*)
    {
        ReturnSFXToPool(NewComp);
    });

    ActiveSFXPool.Emplace(NewComp);
    return NewComp;
}


void UECSoundSubsystem::ReturnSFXToPool(UAudioComponent* Comp)
{
    if (!IsValid(Comp))
    {
        return;
    }

    ActiveSFXPool.RemoveSingleSwap(Comp);
    AvailableSFXPool.Emplace(Comp);
}


void UECSoundSubsystem::InitializeSFXPool()
{
    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    AvailableSFXPool.Reserve(PoolConfig.SFXPoolSize);

    for (int32 i = 0; i < PoolConfig.SFXPoolSize; ++i)
    {
        UAudioComponent* NewComp = NewObject<UAudioComponent>(World);
        if (IsValid(NewComp))
        {
            NewComp->bAutoActivate = false;
            NewComp->bAutoDestroy = false;
            NewComp->RegisterComponentWithWorld(World);

            // 재생 완료 시 자동으로 가용 풀에 반환합니다.
            NewComp->OnAudioFinishedNative.AddWeakLambda(this, [this, NewComp](UAudioComponent*)
            {
                ReturnSFXToPool(NewComp);
            });

            AvailableSFXPool.Emplace(NewComp);
        }
    }
    
    UE_LOG(ALogSoundSubsystem, Warning, TEXT("USoundSubsystem::InitializeSFXPool : %d 개 생성했습니다."), PoolConfig.SFXPoolSize);
}


void UECSoundSubsystem::HandleWorldInitializedActors(const UWorld::FActorsInitializedParams& Params)
{
    // 우리 게임인스턴스의 월드일 때만 풀을 재생성합니다. (에디터/PIE의 다른 월드는 무시)
    if (Params.World == GetWorld())
    {
        RebuildSoundPools();
    }
}


void UECSoundSubsystem::RebuildSoundPools()
{
    // 이전 월드의 잔여 풀 상태를 정리합니다. 파괴된 컴포넌트는 GC가 처리합니다.
    BGMPool.Empty();
    ActiveSFXPool.Empty();
    AvailableSFXPool.Empty();
    CurrentBGMIndex = 0;
    NextBGMIndex = 1;
    FadeState = EBGMFadeState::None;

    InitializeBGMPool();
    InitializeSFXPool();
}


void UECSoundSubsystem::InitializeBGMPool()
{
    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    BGMPool.Reserve(PoolConfig.BGMPoolSize);

    for (int32 i = 0; i < PoolConfig.BGMPoolSize; ++i)
    {
        UAudioComponent* NewComp = NewObject<UAudioComponent>(World);
        if (IsValid(NewComp))
        {
            NewComp->bAutoActivate = false;
            NewComp->bAutoDestroy = false;
            NewComp->bIsMusic = true;
            NewComp->RegisterComponentWithWorld(World);
            BGMPool.Emplace(NewComp);
        }
    }
    
    UE_LOG(ALogSoundSubsystem, Warning, TEXT("USoundSubsystem::InitializeBGMPool : %d 개 생성했습니다."), PoolConfig.BGMPoolSize);
}


USoundBase* UECSoundSubsystem::FindSound(FName RowName) const
{
    if (!IsValid(SoundDataTable) || RowName.IsNone())
    {
        UE_LOG(ALogSoundSubsystem, Warning, TEXT("USoundSubsystem::FindSound : %s 사운드를 찾을 수 없습니다."), *RowName.ToString());
        return nullptr;
    }

    const FSoundTableRow* Row = SoundDataTable->FindRow<FSoundTableRow>(RowName, TEXT("USoundSubsystem::FindSound"));
    return Row ? Row->Sound : nullptr;
}


void UECSoundSubsystem::PlayBGMByName(FName RowName)
{
    PlayBGM(FindSound(RowName));
}


void UECSoundSubsystem::PlayBGMWithFadeByName(FName RowName, float FadeInDuration)
{
    PlayBGMWithFade(FindSound(RowName), FadeInDuration);
}


void UECSoundSubsystem::CrossFadeBGMByName(FName RowName, float CrossFadeDuration)
{
    CrossFadeBGM(FindSound(RowName), CrossFadeDuration);
}


UAudioComponent* UECSoundSubsystem::PlaySFXByName(FName RowName)
{
    return PlaySFX(FindSound(RowName));
}


UAudioComponent* UECSoundSubsystem::PlaySFXAtLocationByName(FName RowName, const FVector& Location)
{
    return PlaySFXAtLocation(FindSound(RowName), Location);
}


void UECSoundSubsystem::ApplySoundMixModifier()
{
    if (!IsValid(MasterSoundMix))
    {
        return;
    }

    // SoundClass별 볼륨을 Sound Mix를 통해 적용합니다.
    if (IsValid(MasterSoundClass))
    {
        UGameplayStatics::SetSoundMixClassOverride(
            this,
            MasterSoundMix,
            MasterSoundClass,
            MasterVolume,
            1.0f,
            0.0f,
            true
        );
        
        UE_LOG(ALogSoundSubsystem, Warning, TEXT("USoundSubsystem::ApplySoundMixModifier : MasterSoundMix가 설정되었습니다."));
    }

    if (IsValid(BGMSoundClass))
    {
        UGameplayStatics::SetSoundMixClassOverride(
            this,
            MasterSoundMix,
            BGMSoundClass,
            BGMVolume,
            1.0f,
            0.0f,
            true
        );
        
        UE_LOG(ALogSoundSubsystem, Warning, TEXT("USoundSubsystem::ApplySoundMixModifier : BGMSoundClass 설정되었습니다."));
    }

    if (IsValid(SFXSoundClass))
    {
        UGameplayStatics::SetSoundMixClassOverride(
            this,
            MasterSoundMix,
            SFXSoundClass,
            SFXVolume,
            1.0f,
            0.0f,
            true
        );
        
        UE_LOG(ALogSoundSubsystem, Warning, TEXT("USoundSubsystem::ApplySoundMixModifier : SFXSoundClass 설정되었습니다."));
    }
}