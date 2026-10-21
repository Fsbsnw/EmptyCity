#include "ECGameInstance.h"

#include "ECGameplayTags.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/ECCutsceneSubsystem.h"
#include "Subsystem/ECProgressionSubsystem.h"
#include "Subsystem/ECRegionSubsystem.h"
#include "Subsystem/ECSoundSubsystem.h"
#include "Subsystem/ECTimeSubsystem.h"

namespace
{
	const FName MainGameplayLevelName(TEXT("/Game/_EmptyCity/Maps/Main/LV_Main"));
}

void UECGameInstance::Init()
{
	// Super::Init()에서 GameInstanceSubsystem 컬렉션을 생성하기 전에
	// BP_SoundSubsystem 클래스를 메모리에 올립니다.
	if (UClass* LoadedSoundSubsystemClass = SoundSubsystemClass.Get())
	{
		LoadedSoundSubsystemClass->GetDefaultObject();
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("SoundSubsystemClass가 설정되지 않았습니다."));
	}

	// 여기서 로드된 BP_SoundSubsystem이 자동으로 발견·등록됩니다.
	Super::Init();

	// 실제 생성된 클래스 확인용
	if (UECSoundSubsystem* SoundSubsystem =
		UGameInstance::GetSubsystem<UECSoundSubsystem>(this))
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("SoundSubsystem 생성 완료: %s"),
			*SoundSubsystem->GetClass()->GetName());
	}
}

void UECGameInstance::StartDay8MorningDebugGame()
{
#if UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Warning, TEXT("8일차 디버그 시작은 Shipping 빌드에서 사용할 수 없습니다."));
#else
	// LocalPlayerSubsystem은 맵 전환 뒤에도 유지되므로 완료 태그와 활성 재생 상태를 먼저 비웁니다.
	if (ULocalPlayer* LocalPlayer = GetFirstGamePlayer())
	{
		if (UECCutsceneSubsystem* CutsceneSubsystem =
			LocalPlayer->GetSubsystem<UECCutsceneSubsystem>())
		{
			CutsceneSubsystem->ResetForDebugGameStart();
		}
	}

	bPendingDay8MorningDebugStart = true;
	UGameplayStatics::OpenLevel(this, MainGameplayLevelName);
#endif
}

bool UECGameInstance::ApplyPendingDebugGameStart()
{
	if (!bPendingDay8MorningDebugStart)
	{
		return false;
	}

	bPendingDay8MorningDebugStart = false;

	if (UECRegionSubsystem* RegionSubsystem = GetSubsystem<UECRegionSubsystem>())
	{
		RegionSubsystem->ResetForDebugGameStart();
	}

	// 8일차에는 아직 열리지 않아야 하는 9일차 보스 전투 입구를 이전 런타임 상태에서 분리합니다.
	if (UECProgressionSubsystem* ProgressionSubsystem = GetSubsystem<UECProgressionSubsystem>())
	{
		ProgressionSubsystem->LockContent(ECGameplayTags::MapNode_Location_CultistBase_BossBattle);
	}

	if (UECTimeSubsystem* TimeSubsystem = GetSubsystem<UECTimeSubsystem>())
	{
		TimeSubsystem->ApplyTransientDebugTime(8, EECTimeOfDay::Morning);
		UE_LOG(LogTemp, Log, TEXT("세이브 파일 없이 8일차 오전 디버그 상태를 적용했습니다."));
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("TimeSubsystem이 없어 8일차 디버그 상태를 적용하지 못했습니다."));
	return false;
}
