#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "ECSoundSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(ALogSoundSubsystem, Log, All);

class UAudioComponent;
class USoundBase;
class USoundMix;
class USoundClass;
class UDataTable;

/** 사운드 데이터 테이블의 행 구조체입니다. */
USTRUCT(BlueprintType)
struct FSoundTableRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    TObjectPtr<USoundBase> Sound;
};

/** BGM 페이드 상태를 나타내는 열거형입니다. */
UENUM(BlueprintType)
enum class EBGMFadeState : uint8
{
    None,
    FadingIn,
    FadingOut,
    CrossFading
};

/** 사운드 풀의 설정 정보를 담는 구조체입니다. */
USTRUCT(BlueprintType)
struct FSoundPoolConfig
{
    GENERATED_BODY()

    /** BGM 풀의 초기 크기입니다. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "변수")
    int32 BGMPoolSize = 2;

    /** SFX 풀의 초기 크기입니다. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "변수")
    int32 SFXPoolSize = 30;
};

/**
 * 게임 전반의 BGM 및 SFX를 통합 관리하는 사운드 서브시스템입니다.
 * Sound Class / Sound Mix 에셋을 통해 볼륨을 제어하며,
 * 오브젝트 풀링을 활용하여 AudioComponent를 효율적으로 재사용합니다.
 */
UCLASS(Blueprintable)
class EMPTYCITY_API UECSoundSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
    
// ─────────────────────────────────────────────────────────────
// Subsystem Interface
// ─────────────────────────────────────────────────────────────
public:
    /** 서브시스템 초기화 함수입니다. */
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /** 서브시스템 종료 함수입니다. */
    virtual void Deinitialize() override;

    /**
     * 서브시스템 인스턴스 생성 여부를 결정하는 함수입니다.
     * 이 클래스를 상속한 클래스(BP_SoundSubsystem 등)가 있으면
     * 가장 하위 클래스만 인스턴스로 생성됩니다.
     */
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;


    
// ─────────────────────────────────────────────────────────────	
// Singleton	
// ─────────────────────────────────────────────────────────────
public:
    /** WorldContextObject으로 SoundSubsystem을 반환합니다. */
    static UECSoundSubsystem& Get(const UObject* WorldContextObject); 

    
    
// ─────────────────────────────────────────────────────────────
// BGM Function
// ─────────────────────────────────────────────────────────────
public:
    /**
     * BGM을 즉시 재생하는 함수입니다.
     * 기존 BGM은 즉시 중단됩니다.
     */
    UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
    void PlayBGM(USoundBase* NewBGM);

    /**
     * BGM을 페이드 인과 함께 재생하는 함수입니다.
     */
    UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
    void PlayBGMWithFade(USoundBase* NewBGM, float FadeInDuration = 1.0f);

    /** BGM을 즉시 중단하는 함수입니다. */
    UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
    void StopBGM();

    /**
     * BGM을 페이드 아웃과 함께 중단하는 함수입니다.
     */
    UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
    void StopBGMWithFade(float FadeOutDuration = 1.0f);

    /**
     * 현재 BGM에서 새로운 BGM으로 크로스페이드 전환하는 함수입니다.
     */
    UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
    void CrossFadeBGM(USoundBase* NewBGM, float CrossFadeDuration = 2.0f);

    /** 데이터 테이블 행 이름으로 BGM을 즉시 재생하는 함수입니다. */
    UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
    void PlayBGMByName(FName RowName);

    /** 데이터 테이블 행 이름으로 BGM을 페이드 인과 함께 재생하는 함수입니다. */
    UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
    void PlayBGMWithFadeByName(FName RowName, float FadeInDuration = 1.0f);

    /** 데이터 테이블 행 이름으로 크로스페이드 전환하는 함수입니다. */
    UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
    void CrossFadeBGMByName(FName RowName, float CrossFadeDuration = 2.0f);

    
    
// ─────────────────────────────────────────────────────────────
// SFX Function
// ─────────────────────────────────────────────────────────────
public:
    /**
     * 2D SFX를 재생하는 함수입니다.
     */
    UFUNCTION(BlueprintCallable, Category = "Sound|SFX")
    UAudioComponent* PlaySFX(USoundBase* SFXSound);

    /**
     * 3D 위치 기반 SFX를 재생하는 함수입니다.
     */
    UFUNCTION(BlueprintCallable, Category = "Sound|SFX")
    UAudioComponent* PlaySFXAtLocation(USoundBase* SFXSound, const FVector& Location);

    /** 데이터 테이블 행 이름으로 2D SFX를 재생하는 함수입니다. */
    UFUNCTION(BlueprintCallable, Category = "Sound|SFX")
    UAudioComponent* PlaySFXByName(FName RowName);

    /** 데이터 테이블 행 이름으로 3D 위치 기반 SFX를 재생하는 함수입니다. */
    UFUNCTION(BlueprintCallable, Category = "Sound|SFX")
    UAudioComponent* PlaySFXAtLocationByName(FName RowName, const FVector& Location);

    /** 현재 재생 중인 모든 SFX를 중단하는 함수입니다. */
    UFUNCTION(BlueprintCallable, Category = "Sound|SFX")
    void StopAllSFX();

    
// ─────────────────────────────────────────────────────────────
// Volume Function
// ─────────────────────────────────────────────────────────────
public:
    /**
     * 마스터 볼륨을 설정하는 함수입니다.
     */
    UFUNCTION(BlueprintCallable, Category = "Sound|Volume")
    void SetMasterVolume(float Volume);

    /**
     * BGM 볼륨을 설정하는 함수입니다.
     */
    UFUNCTION(BlueprintCallable, Category = "Sound|Volume")
    void SetBGMVolume(float Volume);

    /**
     * SFX 볼륨을 설정하는 함수입니다.
     */
    UFUNCTION(BlueprintCallable, Category = "Sound|Volume")
    void SetSFXVolume(float Volume);

    /** 현재 마스터 볼륨 값을 반환하는 함수입니다. */
    UFUNCTION(BlueprintPure, Category = "Sound|Volume")
    float GetMasterVolume() const { return MasterVolume; }

    /** 현재 BGM 볼륨 값을 반환하는 함수입니다. */
    UFUNCTION(BlueprintPure, Category = "Sound|Volume")
    float GetBGMVolume() const { return BGMVolume; }

    /** 현재 SFX 볼륨 값을 반환하는 함수입니다. */
    UFUNCTION(BlueprintPure, Category = "Sound|Volume")
    float GetSFXVolume() const { return SFXVolume; }
    
    
// ─────────────────────────────────────────────────────────────
// ObjectPooling Function
// ─────────────────────────────────────────────────────────────
private:
    /** SFX 풀을 초기화하는 함수입니다. */
    void InitializeSFXPool();

    /** BGM 풀을 초기화하는 함수입니다. */
    void InitializeBGMPool();

    /** 새 월드의 액터 초기화가 끝났을 때 호출되어 사운드 풀을 재생성합니다. */
    void HandleWorldInitializedActors(const UWorld::FActorsInitializedParams& Params);

    /** BGM/SFX 풀을 현재 월드 기준으로 재생성합니다. */
    void RebuildSoundPools();

    
// ─────────────────────────────────────────────────────────────
// Internal BGM Function
// ─────────────────────────────────────────────────────────────
private:
    /** BGM의 페이드 틱을 처리하는 함수입니다. */
    void TickFade();

    /** BGM 풀에서 사용 가능한 AudioComponent를 가져오는 함수입니다. */
    UAudioComponent* GetAvailableBGMComponent();

    /** 현재 재생 중인 BGM AudioComponent를 반환하는 함수입니다. */
    UAudioComponent* GetCurrentBGMComponent() const;

    /** 크로스페이드용 다음 BGM AudioComponent를 반환하는 함수입니다. */
    UAudioComponent* GetNextBGMComponent() const;

    
// ─────────────────────────────────────────────────────────────
// Internal SFX Function
// ─────────────────────────────────────────────────────────────
private:
    /** SFX 풀에서 사용 가능한 AudioComponent를 가져오는 함수입니다. */
    UAudioComponent* GetAvailableSFXComponent();

    /** 재생이 끝난 SFX AudioComponent를 가용 풀로 반환하는 함수입니다. */
    void ReturnSFXToPool(UAudioComponent* Comp);


// ─────────────────────────────────────────────────────────────
// DataTable Function
// ─────────────────────────────────────────────────────────────
private:
    /** 행 이름으로 사운드 데이터 테이블에서 USoundBase를 조회합니다. */
    USoundBase* FindSound(FName RowName) const;

    
// ─────────────────────────────────────────────────────────────
// Sound Class / Mix Function
// ─────────────────────────────────────────────────────────────
private:  
    /** Sound Mix 볼륨을 적용하는 함수입니다. */
    void ApplySoundMixModifier();

    
    
// ─────────────────────────────────────────────────────────────
// Sound Class / Mix Variable
// ─────────────────────────────────────────────────────────────
private:
    /** 마스터 Sound Class 에셋입니다. */
    UPROPERTY(EditDefaultsOnly, Category = "변수")
    TObjectPtr<USoundClass> MasterSoundClass;

    /** BGM Sound Class 에셋입니다. */
    UPROPERTY(EditDefaultsOnly, Category = "변수")
    TObjectPtr<USoundClass> BGMSoundClass;

    /** SFX Sound Class 에셋입니다. */
    UPROPERTY(EditDefaultsOnly, Category = "변수")
    TObjectPtr<USoundClass> SFXSoundClass;

    /** 볼륨 조절에 사용할 Sound Mix 에셋입니다. */
    UPROPERTY(EditDefaultsOnly, Category = "변수")
    TObjectPtr<USoundMix> MasterSoundMix;

    /** 사운드 데이터 테이블의 에셋 경로입니다.*/
    UPROPERTY(EditDefaultsOnly, Category = "변수")
    FSoftObjectPath SoundDataTablePath;

    /** 런타임에 로드된 사운드 데이터 테이블입니다. */
    UPROPERTY(EditDefaultsOnly, Category = "변수")
    TObjectPtr<UDataTable> SoundDataTable;

    
// ─────────────────────────────────────────────────────────────
// ObjectPooling Variable
// ─────────────────────────────────────────────────────────────
private:
    /** 풀 설정 구조체입니다. */
    UPROPERTY(EditDefaultsOnly, Category = "변수")
    FSoundPoolConfig PoolConfig;
    
    /** BGM용 AudioComponent 풀입니다. */
    UPROPERTY()
    TArray<TObjectPtr<UAudioComponent>> BGMPool;

    /** 재생 대기 중인 SFX AudioComponent 풀입니다. */
    UPROPERTY()
    TArray<TObjectPtr<UAudioComponent>> AvailableSFXPool;

    /** 현재 재생 중인 SFX AudioComponent 풀입니다. */
    UPROPERTY()
    TArray<TObjectPtr<UAudioComponent>> ActiveSFXPool;

    /** 현재 재생 중인 BGM 풀 인덱스입니다. */
    int32 CurrentBGMIndex = 0;

    /** 크로스페이드 대상 BGM 풀 인덱스입니다. */
    int32 NextBGMIndex = 1;

    /** OnWorldInitializedActors 델리게이트 핸들입니다. */
    FDelegateHandle WorldInitializedHandle;
    
    
// ─────────────────────────────────────────────────────────────
// 페이드 관련 변수
// ─────────────────────────────────────────────────────────────
private:
    /** 현재 BGM 페이드 상태입니다. */
    EBGMFadeState FadeState = EBGMFadeState::None;

    /** 페이드 진행 시간입니다. */
    float FadeElapsed = 0.0f;

    /** 페이드 총 지속 시간입니다. */
    float FadeDuration = 1.0f;

    /** 페이드 틱 타이머 핸들입니다. */
    FTimerHandle FadeTimerHandle;

    
// ─────────────────────────────────────────────────────────────
// 볼륨 변수
// ─────────────────────────────────────────────────────────────
private:
    /** 마스터 볼륨 값입니다. */
    float MasterVolume = 1.0f;

    /** BGM 볼륨 값입니다. */
    float BGMVolume = 1.0f;

    /** SFX 볼륨 값입니다. */
    float SFXVolume = 1.0f;
};