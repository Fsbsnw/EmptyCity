#pragma once

#include "CoreMinimal.h"
#include "Data/Workbench/ECWorkbenchUpgradeTypes.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECWorkbenchToolUpgradeWidget.generated.h"

class UButton;
class UImage;
class UPanelWidget;
class UTexture2D;
class UWorkbenchToolUpgradeViewModel;
class UECWorkbenchModelPreviewWidget;
class UECWorkbenchUpgradeRequirementWidget;

/** 도구 탭의 Axe/Pickaxe 선택, 미리보기와 강화 실행을 관리합니다. */
UCLASS()
class EMPTYCITY_API UECWorkbenchToolUpgradeWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	virtual void InjectViewModel(UMVVMViewModelBase* TargetViewModel) override;

	UFUNCTION(BlueprintCallable, Category = "Workbench|Tool Upgrade")
	void SelectTool(EECWorkbenchUpgradeType NewTool);

	UFUNCTION(BlueprintCallable, Category = "Workbench|Tool Upgrade")
	void RefreshToolPreview();

	UFUNCTION(BlueprintPure, Category = "Workbench|Tool Upgrade")
	EECWorkbenchUpgradeType GetSelectedTool() const { return SelectedTool; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	/** 선택 또는 강화로 3D 미리보기를 갱신해야 할 때 호출됩니다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Workbench|Tool Upgrade")
	void OnToolPreviewChanged(EECWorkbenchUpgradeType ToolType, int32 CurrentLevel);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Tool Upgrade")
	EECWorkbenchUpgradeType SelectedTool = EECWorkbenchUpgradeType::Axe;

private:
	void InitializeViewModel();
	void RefreshWidgetState();
	void RebuildRequirementWidgets();
	void ApplyLevelImage(int32 CurrentLevel);

	UFUNCTION()
	void HandleUpgradeClicked();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> Button_Upgrade;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UPanelWidget> Panel_RequiredItems;

	/** 현재 도구 강화 단계의 이미지를 표시합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> Image_Background;

	/** 현재 도구 강화 단계의 실제 Static Mesh 프리뷰입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UECWorkbenchModelPreviewWidget> ModelPreviewWidget;

	/** 이 위젯이 담당하는 도구의 단계별 이미지입니다. Axe/Pickaxe WBP에서 각각 지정합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Workbench|Tool Upgrade|Preview", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> Level1Image;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Workbench|Tool Upgrade|Preview", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> Level2Image;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Workbench|Tool Upgrade|Preview", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> Level3Image;

	UPROPERTY(EditDefaultsOnly, Category = "Workbench|Tool Upgrade")
	TSubclassOf<UECWorkbenchUpgradeRequirementWidget> RequirementWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UWorkbenchToolUpgradeViewModel> ToolUpgradeViewModel;
};
