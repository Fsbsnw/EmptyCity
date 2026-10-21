#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/ECUserWidget.h"
#include "ECWorkbenchWeaponUpgradeWidget.generated.h"

class UButton;
class UImage;
class UPanelWidget;
class UTexture2D;
class UWorkbenchWeaponUpgradeViewModel;
class UECWorkbenchModelPreviewWidget;
class UECWorkbenchUpgradeRequirementWidget;

/** 무기 단계 미리보기, 강화 비용 표시와 강화 실행을 관리합니다. */
UCLASS()
class EMPTYCITY_API UECWorkbenchWeaponUpgradeWidget : public UECUserWidget
{
	GENERATED_BODY()

public:
	virtual void InjectViewModel(UMVVMViewModelBase* TargetViewModel) override;

	UFUNCTION(BlueprintCallable, Category = "Workbench|Weapon Upgrade")
	void RefreshWeaponPreview();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	/** 단계가 변경되어 3D 무기 미리보기를 갱신해야 할 때 호출됩니다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Workbench|Weapon Upgrade")
	void OnWeaponPreviewChanged(int32 CurrentLevel);

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

	/** 현재 무기 강화 단계의 이미지를 표시합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> Image_Background;

	/** 현재 무기 강화 단계의 실제 Static Mesh 프리뷰입니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UECWorkbenchModelPreviewWidget> ModelPreviewWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Workbench|Weapon Upgrade|Preview", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> Level1Image;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Workbench|Weapon Upgrade|Preview", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> Level2Image;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Workbench|Weapon Upgrade|Preview", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> Level3Image;

	UPROPERTY(EditDefaultsOnly, Category = "Workbench|Weapon Upgrade")
	TSubclassOf<UECWorkbenchUpgradeRequirementWidget> RequirementWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UWorkbenchWeaponUpgradeViewModel> WeaponUpgradeViewModel;
};
