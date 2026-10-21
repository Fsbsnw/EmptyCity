#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "PlayerCoinViewModel.generated.h"

class AECPlayerState;

/** PlayerState에 저장된 재화 데이터를 UI에 제공하는 ViewModel입니다. */
UCLASS()
class EMPTYCITY_API UPlayerCoinViewModel : public UECViewModelBase
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies(AActor* ContextActor) override;
	virtual void BroadcastInitialValues() override;
	virtual void BeginDestroy() override;

	int32 GetCoinCount() const { return CoinCount; }
	void SetCoinCount(int32 NewCoinCount);

	/** TextBlock.Text에 직접 바인딩할 수 있는 Coin 수량 텍스트입니다. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Currency")
	FText GetCoinCountText() const;

	/** 현재 Coin으로 지정한 금액을 지불할 수 있는지 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Currency")
	bool CanAfford(int32 Cost) const;

private:
	void HandleCoinChanged(int32 NewCoin);
	void RefreshCoinCount();

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"))
	int32 CoinCount = 0;

	UPROPERTY()
	TWeakObjectPtr<AECPlayerState> PlayerState;
};
