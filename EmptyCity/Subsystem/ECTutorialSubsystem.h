#pragma once

#include "CoreMinimal.h"
#include "Save/ECSaveable.h"
#include "Subsystem/ECGameplayMessageSubsystem.h"
#include "Subsystem/ECTutorialTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ECTutorialSubsystem.generated.h"

struct FECTutorialInputMessage;
class UECTutorialWidget;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_Tutorial);

/** 튜토리얼 순서, 복합 입력 조건, 저장 상태를 관리합니다. */
UCLASS()
class EMPTYCITY_API UECTutorialSubsystem : public ULocalPlayerSubsystem, public IECSaveable
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void WriteSaveData(FECGameplaySaveData& InOutSaveData) const override;
	virtual void ReadSaveData(const FECGameplaySaveData& SaveData) override;

	/** UIManager와 HUD가 준비된 뒤 튜토리얼 위젯을 HUD 레이어에 생성합니다. */
	void InitializeTutorialUI();

	/** 세이브 없는 디버그 시작 또는 새 게임에서 첫 단계로 되돌립니다. */
	UFUNCTION(BlueprintCallable, Category="Tutorial")
	void ResetTutorial();

	UFUNCTION(BlueprintPure, Category="Tutorial")
	EECTutorialStage GetCurrentStage() const { return CurrentStage; }

private:
	void HandleTutorialInput(FGameplayTag Channel, const FECTutorialInputMessage& Message);
	void HandleRegionChanged(FGameplayTag PreviousRegionTag, FGameplayTag NewRegionTag);

	void SetStage(EECTutorialStage NewStage);
	void RefreshTutorialWidget();

private:
	UPROPERTY(Transient)
	TObjectPtr<UECTutorialWidget> TutorialWidget;

	EECTutorialStage CurrentStage = EECTutorialStage::Movement;
	bool bSprintUsed = false;
	bool bAttackUsed = false;
	bool bGuardUsed = false;

	FECGameplayMessageListenerHandle TutorialInputListenerHandle;
};

