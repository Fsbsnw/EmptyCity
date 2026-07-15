#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/IECInventory.h"
#include "ECPlayerController.generated.h"

class UECIndicatorManagerComponent;
struct FInteractionOption;
class UECQuickBarComponent;
class UECInventoryManagerComponent;
class UECInputConfig;
class UUIConfigDataAsset;

UCLASS()
class EMPTYCITY_API AECPlayerController : public APlayerController, public IIECInventory
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
// 인벤토리 인터페이스
// ─────────────────────────────────────────────────────────────
public:
	/** 플레이어의 인벤토리를 관리하는 컴포넌트를 반환합니다. (아이템 줍기/보관 시 사용) */
	virtual UECInventoryManagerComponent* GetInventoryManagerComponent() const override;

protected:
	/** 플레이어가 보유한 아이템 전체를 관리하는 인벤토리 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UECInventoryManagerComponent> InventoryManagerComponent;

	/** HUD QuickBar 슬롯과 Pawn 장비 장착을 연결하는 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UECQuickBarComponent> QuickBarComponent;
	
// ─────────────────────────────────────────────────────────────
// UI 라이프사이클 제어
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
	// UI 전담 입력 처리 함수
	void Input_UIAction(FGameplayTag InputTag);
	
// ─────────────────────────────────────────────────────────────
// Indicator Component
// ─────────────────────────────────────────────────────────────
	
public:
	UECIndicatorManagerComponent* GetIndicatorManagerComponent() const;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UECIndicatorManagerComponent> IndicatorManagerComponent;

// ─────────────────────────────────────────────────────────────
// 인게임 연출 제어
// ─────────────────────────────────────────────────────────────
public:
	/** 영상 재생 시에 플레이어 캐릭터의 입력을 비활성화합니다. */
	void SetCinematicInputLocked(bool bIsLocked);
};
