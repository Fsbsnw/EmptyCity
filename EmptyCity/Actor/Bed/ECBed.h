// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/UI/UIConfigDataAsset.h"
#include "GameFramework/Actor.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionOption.h"
#include "ECBed.generated.h"

class UUIManagerSubsystem;
class UBoxComponent;
class FInteractionOptionBuilder;
struct FInteractionQuery;

/**
 * @brief 게임 내 침대 상호작용 및 수면 시퀀스를 관리하는 액터 클래스입니다.
 * 침대 메뉴의 Sleep 버튼을 누르면 페이드 전환 중 다음 날 아침으로 변경합니다.
 */
UCLASS()
class EMPTYCITY_API AECBed : public AActor, public IInteractableTarget
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// InteractableTarget Interface
// ─────────────────────────────────────────────────────────────		
public:
	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& OptionBuilder) override;

	
// ─────────────────────────────────────────────────────────────
// Interaction
// ─────────────────────────────────────────────────────────────	
protected:
	UPROPERTY(EditAnywhere, Category = "변수|상호작용")
	FInteractionOption Option;
	
public:	
	AECBed();

	virtual void BeginPlay() override;

// ============================================================================
// 침대 씬 컴포넌트
// ============================================================================
protected:
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* Mesh;
	
	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* BoxCollision;
	

// ============================================================================
// 수면 시퀀스 및 페이드 연출
// ============================================================================
public:	
	/** 침대 메뉴의 Sleep 버튼에서 호출되어 페이드 전환과 다음 날 처리를 시작합니다. */
	void StartSleepSequence();

	/** 화면이 완전히 검게 변했을 때 다음 날 아침으로 변경합니다. */
	void OnFadeOutCompleted();

	/** 화면이 다시 밝아졌을 때(페이드 인 완료) 호출되어 수면 상태를 초기화하고 플레이어 조작을 복구합니다. */
	void OnFadeInCompleted();

// ============================================================================
// 수면 단계별 UI 제어
// ============================================================================
public:
	/** BedMenu 위젯을 보여줍니다. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ShowBedMenu(APlayerController* PC);

	/** 수면을 실행하지 않고 현재 침대 메뉴만 닫습니다. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CloseBedMenu();

	/** BedMenu 위젯을 닫습니다. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HideBedMenu(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	/** 침대와 상호작용 시에 표시할 메뉴 UI 위젯의 설정 정보입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Bed|UI")
	FUIConfigWidget BedMenuWidgetConfig;

	/** 수면 화면의 페이드 재생 속도입니다. */
	UPROPERTY(EditAnywhere, Category = "Bed|Settings", meta = (ClampMin = "0.01"))
	float SleepFadeSpeed = 0.5f;

private:
	bool bSleepSequenceInProgress = false;

	/** 현재 침대 UI 및 수면 시퀀스를 진행 중인 로컬 플레이어입니다. */
	TWeakObjectPtr<APlayerController> ActivePlayerController;

	UUIManagerSubsystem* GetActiveUIManager() const;
};
