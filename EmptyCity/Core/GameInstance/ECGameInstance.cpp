#include "ECGameInstance.h"

#include "Subsystem/ECSoundSubsystem.h"

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
