// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/ECViewModelBase.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "UI/Widget/ECUserWidget.h"

void UECViewModelBase::BindCallbacksToDependencies(AActor* ContextActor)
{
	CachedContextActor = ContextActor;
	CachedPlayerController.Reset();
	CachedPlayerPawn.Reset();
	CachedPlayerState.Reset();

	if (const UECUserWidget* OwningWidget = GetTypedOuter<UECUserWidget>())
	{
		CachedPlayerController = OwningWidget->GetOwningPlayer();
	}

	if (!CachedPlayerController.IsValid())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(ContextActor))
		{
			CachedPlayerController = PlayerController;
		}
		else if (const APawn* Pawn = Cast<APawn>(ContextActor))
		{
			CachedPlayerController = Cast<APlayerController>(Pawn->GetController());
		}
	}

	if (APlayerController* PlayerController = CachedPlayerController.Get())
	{
		CachedPlayerPawn = PlayerController->GetPawn();
		CachedPlayerState = PlayerController->PlayerState;
	}
}

void UECViewModelBase::BroadcastInitialValues()
{
}
