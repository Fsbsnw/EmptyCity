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
// 플레이어의 수면 진행 단계를 나타냅니다.
UENUM(BlueprintType)
enum class ESleepPhase : uint8
{
	/** 수면 중이 아님 (대기 상태) */
	None,           
	/** 수면 중 꿈 UI가 화면에 출력되고 있는 상태 */
	Dreaming,       
	/** 꿈이 끝났거나 꿈을 꾸지 않아 숙면 메시지 UI가 출력되고 있는 상태 */
	SleepMessage
};

/**
 * @brief 게임 내 침대 상호작용 및 수면 시퀀스를 관리하는 액터 클래스입니다.
 * 화면 페이드 효과와 연계하여 확률에 따른 꿈 연출(Dream UI)과 
 * 숙면 결과 메시지(Sleep UI)를 순차적으로 처리합니다.
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
	/** 상호작용 시 수면 시퀀스를 시작합니다. (화면 페이드 아웃 트리거) */
	void StartSleepSequence();

	/** 수면 시퀀스를 종료하기 위해 화면 페이드 인을 트리거합니다. */
	void EndSleepSequence();
    
	/** 화면이 완전히 검게 변했을 때(페이드 아웃 완료) 호출되어 확률에 따라 꿈 또는 숙면 UI를 띄웁니다. */
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

	/** BedMenu 위젯을 닫습니다. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HideBedMenu(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	/** 수면 중 꿈 연출 UI를 뷰포트에 출력합니다. */
	void ShowDreamUI();

	/** 출력된 꿈 연출 UI를 닫고 다음 단계(숙면 메시지)로 전환합니다. */
	void HideDreamUI();
    
	/** 수면 결과를 알려주는 숙면 메시지 UI를 뷰포트에 출력합니다. */
	void ShowSleepUI();

	/** 숙면 메시지 UI를 닫고 기상 연출(페이드 인)을 준비합니다. */
	void HideSleepUI();

	/** 수면 진행 중 사용자의 AnyKey 입력을 받아,
	 * 현재 수면 단계에 맞춰 다음 시퀀스로 넘기거나 UI를 닫는 전담 델리게이트 함수입니다. */
	UFUNCTION()
	void OnWakeUpInputReceived();

// ============================================================================
// 꿈/수면 설정 및 데이터
// ============================================================================
public:
	/** 침대와 상호작용 시에 표시할 메뉴 UI 위젯의 설정 정보입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Bed|UI")
	FUIConfigWidget BedMenuWidgetConfig;
	
	/** 수면 중 꿈 연출 시 사용할 UI 위젯의 설정 정보입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Bed|UI")
	FUIConfigWidget DreamWidgetConfig;

	/** 기상 직전 숙면 결과를 보여줄 UI 위젯의 설정 정보입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Bed|UI")
	FUIConfigWidget SleepWidgetConfig;

	/** 수면 시 꿈을 꿀 확률입니다. (단위: 백분율, 0.0 ~ 100.0) */
	UPROPERTY(EditAnywhere, Category = "Bed|Settings", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float DreamChance = 50.f;

private:
	/** 현재 진행 중인 수면 시퀀스 상태를 추적하는 내부 변수입니다. */
	ESleepPhase CurrentSleepPhase = ESleepPhase::None;

	/** 현재 침대 UI 및 수면 시퀀스를 진행 중인 로컬 플레이어입니다. */
	TWeakObjectPtr<APlayerController> ActivePlayerController;

	UUIManagerSubsystem* GetActiveUIManager() const;
};
