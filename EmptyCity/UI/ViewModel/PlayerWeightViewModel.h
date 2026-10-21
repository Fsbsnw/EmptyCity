#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "PlayerWeightViewModel.generated.h"

class UECInventoryManagerComponent;

/** 플레이어 인벤토리의 총 무게를 HUD 표시 데이터로 변환하는 ViewModel입니다. */
UCLASS()
class EMPTYCITY_API UPlayerWeightViewModel : public UECViewModelBase
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies(AActor* ContextActor) override;
	virtual void BroadcastInitialValues() override;
	virtual void BeginDestroy() override;

	float GetCurrentWeight() const { return CurrentWeight; }
	void SetCurrentWeight(float NewWeight);

	float GetMaxWeight() const { return MaxWeight; }
	void SetMaxWeight(float NewWeight);

	/** ProgressBar.Percent에 사용할 0~1 범위의 무게 비율입니다. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Weight")
	float GetWeightPercent() const;

private:
	void HandleInventoryUpdated();
	void HandleMaxInventoryWeightChanged(float NewMaxInventoryWeight);
	void RefreshWeight();

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"))
	float CurrentWeight = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	float MaxWeight = 0.0f;

	UPROPERTY()
	TWeakObjectPtr<UECInventoryManagerComponent> InventoryComponent;
};
