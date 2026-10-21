#include "UI/ViewModel/PlayerDeathViewModel.h"

#include "Character/Enemy/ECEnemyCharacterBase.h"
#include "Data/Item/ECItemTableRow.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Subsystem/ECEnemyDataSubsystem.h"
#include "Subsystem/ECGameFlowSubsystem.h"
#include "Subsystem/ECItemDataSubsystem.h"

namespace
{
	AECEnemyCharacterBase* ResolveEnemyActor(AActor* ContextActor)
	{
		if (!ContextActor)
		{
			return nullptr;
		}

		if (AECEnemyCharacterBase* Enemy = Cast<AECEnemyCharacterBase>(ContextActor))
		{
			return Enemy;
		}

		if (AECEnemyCharacterBase* Enemy = Cast<AECEnemyCharacterBase>(ContextActor->GetInstigator()))
		{
			return Enemy;
		}

		return Cast<AECEnemyCharacterBase>(ContextActor->GetOwner());
	}
}

void UPlayerDeathViewModel::BindCallbacksToDependencies(AActor* ContextActor)
{
	Super::BindCallbacksToDependencies(ContextActor);

	const UWorld* World = GetWorld();
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	const UECGameFlowSubsystem* GameFlowSubsystem = GameInstance
		? GameInstance->GetSubsystem<UECGameFlowSubsystem>()
		: nullptr;
	const FECPlayerDeathMessage* DeathMessage = GameFlowSubsystem
		? &GameFlowSubsystem->GetCurrentPlayerDeathMessage()
		: nullptr;
	AActor* DamageInstigator = DeathMessage && IsValid(DeathMessage->DamageInstigator)
		? DeathMessage->DamageInstigator.Get()
		: ContextActor;

	AECEnemyCharacterBase* KillerEnemy = ResolveEnemyActor(DamageInstigator);
	if (!KillerEnemy)
	{
		SetKillerEnemyTag(FGameplayTag());
		SetDeathScreenImage(nullptr);
	}
	else
	{
		UECEnemyDataSubsystem& EnemyData = UECEnemyDataSubsystem::Get(this);
		SetKillerEnemyTag(EnemyData.FindEnemyTag(KillerEnemy->GetClass()));
		SetDeathScreenImage(EnemyData.FindPlayerDeathScreenImage(KillerEnemy->GetClass()));
	}

	const FECItemTableRow* DroppedItemData = DeathMessage
		? UECItemDataSubsystem::Get(this).FindItemData(DeathMessage->DroppedItemDefinition)
		: nullptr;
	if (DroppedItemData && DeathMessage->DroppedItemCount > 0)
	{
		SetDroppedItemIcon(DroppedItemData->Icon.LoadSynchronous());
		SetDroppedItemName(DroppedItemData->DisplayName);
		SetDroppedItemCount(DeathMessage->DroppedItemCount);
	}
	else
	{
		SetDroppedItemIcon(nullptr);
		SetDroppedItemName(FText::GetEmpty());
		SetDroppedItemCount(0);
	}
}

void UPlayerDeathViewModel::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(DeathScreenImage);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(KillerEnemyTag);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(DroppedItemIcon);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(DroppedItemName);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(DroppedItemCount);
}

void UPlayerDeathViewModel::SetDeathScreenImage(UTexture2D* NewImage)
{
	UE_MVVM_SET_PROPERTY_VALUE(DeathScreenImage, NewImage);
}

void UPlayerDeathViewModel::SetKillerEnemyTag(FGameplayTag NewEnemyTag)
{
	UE_MVVM_SET_PROPERTY_VALUE(KillerEnemyTag, NewEnemyTag);
}

void UPlayerDeathViewModel::SetDroppedItemIcon(UTexture2D* NewIcon)
{
	UE_MVVM_SET_PROPERTY_VALUE(DroppedItemIcon, NewIcon);
}

void UPlayerDeathViewModel::SetDroppedItemName(const FText& NewName)
{
	UE_MVVM_SET_PROPERTY_VALUE(DroppedItemName, NewName);
}

void UPlayerDeathViewModel::SetDroppedItemCount(int32 NewCount)
{
	UE_MVVM_SET_PROPERTY_VALUE(DroppedItemCount, NewCount);
}
