#include "Animation/Notify/ECAnimNotify_FarmingSFX.h"

#include "Character/Player/ECPlayer.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Subsystem/ECSoundSubsystem.h"

UECAnimNotify_FarmingSFX::UECAnimNotify_FarmingSFX(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
	NotifyColor = FColor(60, 180, 100, 255);
#endif
}

void UECAnimNotify_FarmingSFX::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp))
	{
		return;
	}

	const AECPlayer* Player = Cast<AECPlayer>(MeshComp->GetOwner());
	if (!IsValid(Player))
	{
		return;
	}

	const FDataTableRowHandle& FarmingSound = Player->GetActiveFarmingSound();
	const FSoundTableRow* SoundData = FarmingSound.GetRow<FSoundTableRow>(
		TEXT("UECAnimNotify_FarmingSFX::Notify"));
	if (!SoundData || !IsValid(SoundData->Sound))
	{
		return;
	}

	UWorld* World = MeshComp->GetWorld();
	if (!IsValid(World) || !World->IsGameWorld())
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	UECSoundSubsystem* SoundSubsystem =
		IsValid(GameInstance) ? GameInstance->GetSubsystem<UECSoundSubsystem>() : nullptr;
	if (!IsValid(SoundSubsystem))
	{
		return;
	}

	SoundSubsystem->PlaySFXAtLocation(SoundData->Sound, MeshComp->GetComponentLocation());
}

FString UECAnimNotify_FarmingSFX::GetNotifyName_Implementation() const
{
	return TEXT("EC Farming SFX");
}
