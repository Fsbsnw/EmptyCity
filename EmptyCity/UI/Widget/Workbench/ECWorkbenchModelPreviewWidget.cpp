#include "UI/Widget/Workbench/ECWorkbenchModelPreviewWidget.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Viewport.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "PreviewScene.h"
#include "Rendering/DrawElements.h"
#include "Slate/SceneViewport.h"
#include "Widgets/SViewport.h"

/** UViewport와 달리 렌더 타깃 알파 블렌딩을 활성화한 작업대 전용 프리뷰 Slate입니다. */
class SECWorkbenchPreviewViewport : public SViewport
{
public:
	SLATE_BEGIN_ARGS(SECWorkbenchPreviewViewport) {}
	SLATE_END_ARGS()

	SECWorkbenchPreviewViewport()
		: PreviewScene(
			FPreviewScene::ConstructionValues()
				.SetEditor(false)
				.SetCreatePhysicsScene(false))
	{
	}

	void Construct(const FArguments& InArgs)
	{
		SViewport::FArguments ViewportArguments;
		ViewportArguments.IgnoreTextureAlpha(false);
		ViewportArguments.EnableBlending(true);
		ViewportArguments.PreMultipliedAlpha(false);
		SViewport::Construct(ViewportArguments);

		ViewportClient = MakeShared<FUMGViewportClient>(&PreviewScene);
		SceneViewport = MakeShared<FSceneViewport>(ViewportClient.Get(), SharedThis(this));
		SetViewportInterface(SceneViewport.ToSharedRef());
	}

	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override
	{
		const bool bEnabled = ShouldBeEnabled(bParentEnabled);
		const TSharedPtr<ISlateViewport> ViewportInterfacePin = ViewportInterface.Pin();

		if (ViewportInterfacePin.IsValid())
		{
			ViewportInterfacePin->OnDrawViewport(
				AllottedGeometry,
				MyCullingRect,
				OutDrawElements,
				LayerId,
				InWidgetStyle,
				bParentEnabled);

			if (!ShouldRenderDirectly() && ViewportInterfacePin->GetViewportRenderTargetTexture())
			{
				// Unreal의 SceneColor 알파는 배경 가시도(1=배경, 0=불투명 물체)라서
				// Slate에 그릴 때 뒤집어야 메시 영역만 일반 알파로 합성됩니다.
				FSlateDrawElement::MakeViewport(
					OutDrawElements,
					LayerId,
					AllottedGeometry.ToPaintGeometry(),
					ViewportInterfacePin,
					ESlateDrawEffect::InvertAlpha,
					InWidgetStyle.GetColorAndOpacityTint());
			}
		}

		return SCompoundWidget::OnPaint(
			Args,
			AllottedGeometry,
			MyCullingRect,
			OutDrawElements,
			LayerId,
			InWidgetStyle,
			bEnabled);
	}

	virtual void Tick(
		const FGeometry& AllottedGeometry,
		const double InCurrentTime,
		const float InDeltaTime) override
	{
		SceneViewport->Invalidate();
		SceneViewport->Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
		ViewportClient->Tick(InDeltaTime);
	}

	FPreviewScene PreviewScene;
	TSharedPtr<FUMGViewportClient> ViewportClient;
	TSharedPtr<FSceneViewport> SceneViewport;
};

TSharedRef<SWidget> UECWorkbenchModelPreviewWidget::RebuildWidget()
{
	PreviewSlate = SNew(SECWorkbenchPreviewViewport);
	return PreviewSlate.ToSharedRef();
}

void UECWorkbenchModelPreviewWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	PreviewSlate.Reset();
}

void UECWorkbenchModelPreviewWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (PreviewSlate.IsValid() && PreviewSlate->ViewportClient.IsValid())
	{
		FEngineShowFlags ShowFlags(ESFIM_Game);
		ShowFlags.DisableAdvancedFeatures();
		PreviewSlate->ViewportClient->SetEngineShowFlags(ShowFlags);
		PreviewSlate->ViewportClient->SetBackgroundColor(BackgroundColor);
		PreviewSlate->PreviewScene.SetLightDirection(LightRotation);
		PreviewSlate->PreviewScene.SetLightBrightness(LightIntensity);
		PreviewSlate->PreviewScene.SetSkyBrightness(SkyIntensity);
		if (PreviewSlate->PreviewScene.DirectionalLight)
		{
			PreviewSlate->PreviewScene.DirectionalLight->SetCastShadows(false);
		}
		PreviewSlate->PreviewScene.UpdateCaptureContents();
	}

	EnsurePreviewActor();
	ApplyPreviewMesh();
}

void UECWorkbenchModelPreviewWidget::NativeDestruct()
{
	PreviewActor = nullptr;
	Super::NativeDestruct();
}

void UECWorkbenchModelPreviewWidget::SetPreviewLevel(int32 NewLevel)
{
	PreviewLevel = FMath::Clamp(NewLevel, 1, 3);
	EnsurePreviewActor();
	ApplyPreviewMesh();
}

void UECWorkbenchModelPreviewWidget::EnsurePreviewActor()
{
	if (IsValid(PreviewActor) || !PreviewSlate.IsValid() || !PreviewSlate->PreviewScene.GetWorld())
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PreviewActor = PreviewSlate->PreviewScene.GetWorld()->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParameters);
	if (PreviewActor && PreviewActor->GetStaticMeshComponent())
	{
		UStaticMeshComponent* MeshComponent = PreviewActor->GetStaticMeshComponent();
		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetCastShadow(false);
		PreviewActor->SetActorEnableCollision(false);
	}
}

void UECWorkbenchModelPreviewWidget::ApplyPreviewMesh()
{
	if (!PreviewSlate.IsValid() || !PreviewSlate->ViewportClient.IsValid() || !IsValid(PreviewActor))
	{
		return;
	}

	UStaticMeshComponent* MeshComponent = PreviewActor->GetStaticMeshComponent();
	UStaticMesh* TargetMesh = ResolveLevelMesh();
	if (!MeshComponent || !TargetMesh)
	{
		return;
	}

	MeshComponent->SetStaticMesh(TargetMesh);
	MeshComponent->SetRelativeRotation(PreviewRotation);
	MeshComponent->SetRelativeScale3D(FVector(FMath::Max(0.01f, PreviewScale)));

	const FBoxSphereBounds MeshBounds = TargetMesh->GetBounds();
	MeshComponent->SetRelativeLocation(-MeshBounds.Origin * PreviewScale);
	const float FramingDistance = FMath::Max(50.0f, MeshBounds.SphereRadius * PreviewScale * 2.2f);
	PreviewSlate->ViewportClient->SetViewLocation(FVector(FramingDistance, 0.0f, 0.0f));
	PreviewSlate->ViewportClient->SetViewRotation(FRotator(0.0f, 180.0f, 0.0f));
}

UStaticMesh* UECWorkbenchModelPreviewWidget::ResolveLevelMesh() const
{
	switch (FMath::Clamp(PreviewLevel, 1, 3))
	{
	case 1:
		return Level1Mesh;
	case 2:
		return Level2Mesh;
	case 3:
		return Level3Mesh;
	default:
		return nullptr;
	}
}
