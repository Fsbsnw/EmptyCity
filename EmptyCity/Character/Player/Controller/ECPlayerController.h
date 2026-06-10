#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerController.h"
#include "ECPlayerController.generated.h"

class UECInputConfig;
class UUIConfigDataAsset;

UCLASS()
class EMPTYCITY_API AECPlayerController : public APlayerController
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// PlayerController Interface
// ─────────────────────────────────────────────────────────────
public:
	AECPlayerController();
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

	virtual void BeginPlay() override;

	
// ─────────────────────────────────────────────────────────────
// UI 및 HUD 라이프사이클 제어
// ─────────────────────────────────────────────────────────────
protected:
	/** 게임 진입 시 GameState의 데이터를 바탕으로 UI 시스템 전체를 총괄하여 초기화하는 함수입니다. */
	void InitializeUI();

	/** 플레이어에게 부여할 UI 입력 액션입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|입력")
	TObjectPtr<UECInputConfig> UIInputConfig;

	/** 플레이어에게 부여할 UI InputMappingContext입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|입력")
	TObjectPtr<UInputMappingContext> UIInputMappingContext;

private:
	/** 메인 화면 오버레이(HUD) 위젯을 생성하고 초기화합니다. */
	void InitializeHUD(UUIConfigDataAsset* ConfigAsset);

	/** 모든 UI 위젯의 클래스를 총괄하는 데이터 에셋을 전달합니다. */
	void InitializeUIManager(UUIConfigDataAsset* ConfigAsset);

	// UI 전담 입력 처리 함수
	void Input_UIAction(const FInputActionValue& Value, FGameplayTag InputTag);
};
