#include "UI/ViewModel/PlayerCoinViewModel.h"

#include "Character/Player/Controller/ECPlayerController.h"
#include "Character/Player/State/ECPlayerState.h"

void UPlayerCoinViewModel::BindCallbacksToDependencies(AActor* ContextActor)
{
	Super::BindCallbacksToDependencies(ContextActor);

	if (AECPlayerState* PreviousPlayerState = PlayerState.Get())
	{
		PreviousPlayerState->OnCoinChanged.RemoveAll(this);
	}

	AECPlayerController* PlayerController = Cast<AECPlayerController>(GetPlayerController());
	PlayerState = PlayerController ? PlayerController->GetPlayerState<AECPlayerState>() : nullptr;

	if (AECPlayerState* CurrentPlayerState = PlayerState.Get())
	{
		CurrentPlayerState->OnCoinChanged.AddUObject(this, &ThisClass::HandleCoinChanged);
	}

	RefreshCoinCount();
}

void UPlayerCoinViewModel::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CoinCount);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCoinCountText);
}

void UPlayerCoinViewModel::BeginDestroy()
{
	if (AECPlayerState* CurrentPlayerState = PlayerState.Get())
	{
		CurrentPlayerState->OnCoinChanged.RemoveAll(this);
	}

	PlayerState.Reset();

	Super::BeginDestroy();
}

void UPlayerCoinViewModel::SetCoinCount(int32 NewCoinCount)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(CoinCount, FMath::Max(0, NewCoinCount)))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCoinCountText);
	}
}

FText UPlayerCoinViewModel::GetCoinCountText() const
{
	return FText::AsNumber(CoinCount);
}

bool UPlayerCoinViewModel::CanAfford(int32 Cost) const
{
	const AECPlayerState* CurrentPlayerState = PlayerState.Get();
	return CurrentPlayerState
		? CurrentPlayerState->CanAfford(Cost)
		: Cost >= 0 && CoinCount >= Cost;
}

void UPlayerCoinViewModel::HandleCoinChanged(int32 NewCoin)
{
	SetCoinCount(NewCoin);
}

void UPlayerCoinViewModel::RefreshCoinCount()
{
	const AECPlayerState* CurrentPlayerState = PlayerState.Get();
	SetCoinCount(CurrentPlayerState ? CurrentPlayerState->GetCoin() : 0);
}
