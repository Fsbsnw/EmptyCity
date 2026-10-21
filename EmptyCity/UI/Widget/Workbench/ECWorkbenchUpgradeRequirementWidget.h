#pragma once

#include "CoreMinimal.h"
#include "Inventory/ECInventoryItemRequirement.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECWorkbenchUpgradeRequirementWidget.generated.h"

class UWorkbenchUpgradeRequirementViewModel;

/** WBP가 강화 재료 한 종류를 표시할 수 있게 ViewModel을 주입하는 재사용 위젯 기반 클래스입니다. */
UCLASS()
class EMPTYCITY_API UECWorkbenchUpgradeRequirementWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Workbench|Upgrade")
	void SetRequirement(const FECInventoryItemRequirement& NewRequirement);

	UFUNCTION(BlueprintPure, Category = "Workbench|Upgrade")
	const FECInventoryItemRequirement& GetRequirement() const { return Requirement; }

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Upgrade", meta = (ExposeOnSpawn = "true"))
	FECInventoryItemRequirement Requirement;

private:
	UPROPERTY(Transient)
	TObjectPtr<UWorkbenchUpgradeRequirementViewModel> RequirementViewModel;
};

