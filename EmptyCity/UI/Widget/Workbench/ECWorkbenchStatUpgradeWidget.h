#pragma once

#include "CoreMinimal.h"
#include "Data/Workbench/ECWorkbenchUpgradeTypes.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECWorkbenchStatUpgradeWidget.generated.h"

class UWorkbenchStatUpgradeViewModel;
class UButton;
class UPanelWidget;
class UECWorkbenchUpgradeRequirementWidget;

/** 스탯 탭에서 항상 표시되는 강화 전/후 카드의 C++ 부모 위젯입니다. */
UCLASS()
class EMPTYCITY_API UECWorkbenchStatUpgradeWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * WBP_Workbench의 OnViewModelSet에서 전달되는 ViewModel을 받습니다.
	 * 스탯 카드 셋이 같은 상태 인스턴스를 공유하지 않도록 카드 소유 인스턴스로 분리합니다.
	 */
	virtual void InjectViewModel(UMVVMViewModelBase* TargetViewModel) override;

	UFUNCTION(BlueprintCallable, Category = "Workbench|Stat Upgrade")
	void SetUpgradeType(EECWorkbenchUpgradeType NewUpgradeType);

	UFUNCTION(BlueprintCallable, Category = "Workbench|Stat Upgrade")
	void RefreshStatPreview();

	UFUNCTION(BlueprintPure, Category = "Workbench|Stat Upgrade")
	EECWorkbenchUpgradeType GetUpgradeType() const { return UpgradeType; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Stat Upgrade", meta = (ExposeOnSpawn = "true"))
	EECWorkbenchUpgradeType UpgradeType = EECWorkbenchUpgradeType::MaxHealth;

private:
	void InitializeViewModel();
	void RefreshCardState();
	void RebuildRequirementWidgets();

	UFUNCTION()
	void HandleUpgradeClicked();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> Button_Upgrade;

	/** 다음 단계의 필요 재료 위젯들이 생성될 패널입니다. Horizontal/Vertical/WrapBox 모두 사용할 수 있습니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UPanelWidget> Panel_RequiredItems;

	/** UECWorkbenchUpgradeRequirementWidget을 부모로 만든 재료 행 WBP를 지정합니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Workbench|Stat Upgrade")
	TSubclassOf<UECWorkbenchUpgradeRequirementWidget> RequirementWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UWorkbenchStatUpgradeViewModel> StatUpgradeViewModel;
};
