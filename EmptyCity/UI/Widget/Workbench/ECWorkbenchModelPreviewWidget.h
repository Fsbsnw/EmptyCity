#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ECWorkbenchModelPreviewWidget.generated.h"

class AStaticMeshActor;
class SECWorkbenchPreviewViewport;
class UStaticMesh;

/**
 * 작업대 도구/무기 상세에 실제 Static Mesh를 표시하는 재사용 가능한 3D 프리뷰입니다.
 * 파생 WBP를 상세 위젯에 ModelPreviewWidget 이름으로 배치하고 레벨별 메시만 지정하면 됩니다.
 */
UCLASS()
class EMPTYCITY_API UECWorkbenchModelPreviewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Workbench|Model Preview")
	void SetPreviewLevel(int32 NewLevel);

	UFUNCTION(BlueprintPure, Category = "Workbench|Model Preview")
	int32 GetPreviewLevel() const { return PreviewLevel; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void EnsurePreviewActor();
	void ApplyPreviewMesh();
	UStaticMesh* ResolveLevelMesh() const;

	/** 상세 위젯에 배치한 인스턴스마다 해당 장비의 1단계 메시를 지정합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Model Preview|Meshes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> Level1Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Model Preview|Meshes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> Level2Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Model Preview|Meshes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> Level3Mesh;

	/** 메시의 로컬 축이 화면과 맞지 않을 때만 조정합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Model Preview|Transform", meta = (AllowPrivateAccess = "true"))
	FRotator PreviewRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Model Preview|Transform", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float PreviewScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Model Preview|Lighting", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float LightIntensity = 8.0f;

	/** 카메라 앞쪽에서 메시를 비추는 기본 조명 방향입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Model Preview|Lighting", meta = (AllowPrivateAccess = "true"))
	FRotator LightRotation = FRotator(-25.0f, 160.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Model Preview|Lighting", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float SkyIntensity = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workbench|Model Preview|Appearance", meta = (AllowPrivateAccess = "true"))
	FLinearColor BackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);

	UPROPERTY(Transient)
	TObjectPtr<AStaticMeshActor> PreviewActor;

	UPROPERTY(Transient)
	int32 PreviewLevel = 1;

	TSharedPtr<SECWorkbenchPreviewViewport> PreviewSlate;
};
