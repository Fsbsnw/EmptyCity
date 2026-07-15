// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/UI/UIConfigDataAsset.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UIManagerSubsystem.generated.h"

struct FECPopupInfo;
class UECPopupWidget;
class UECUserWidget;
class UECMainLayoutWidget;

DECLARE_DELEGATE(FOnFadeEvent);

/**
 * @brief 게임 내 UI 위젯의 전반적인 관리를 담당하는 서브시스템입니다.
 * 위젯의 라이프사이클(생성, 열기, 닫기), 입력 라우팅, 포커스 스택 관리, 
 * 그리고 화면 페이드(Fade) 연출 기능을 제공합니다.
 */
UCLASS()
class EMPTYCITY_API UUIManagerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
public:
	/** 지정한 로컬 플레이어 컨트롤러의 UIManagerSubsystem을 반환합니다. */
	static UUIManagerSubsystem* Get(const APlayerController* PlayerController);
	
public:	
	/** 현재 Map Level에서 사용될 위젯의 정보를 저장합니다. */
	void InitializeUIConfig(const FUIConfigData& UIConfig);

	/** 위젯 토글 요청을 실행합니다. */
	void RequestToggleInput(FGameplayTag Tag, AActor* ContextActor = nullptr);
	
	/** 클릭된 위젯에 맞게 Focus Stack을 업데이트합니다. */
	void UpdateFocusStack(UECUserWidget* FocusedWidget);

// =========================================================================
// 화면 전환 연출
// =========================================================================
public:
	/** 페이드 아웃을 실행합니다.(맵 이동, 혹은 침대 수면 시작 시) */
	void PlayFadeOut(FOnFadeEvent OnFinishedCallback = FOnFadeEvent(), float InFadeSpeed = 1.f);

	/** 페이드 인을 실행합니다. (맵 시작 시, 혹은 수면 종료 시) */
	void PlayFadeIn(FOnFadeEvent OnFinishedCallback = FOnFadeEvent(), float InFadeSpeed = 1.f);

	/** 페이드 아웃/인 결합 (UI가 열리거나 닫힐 때 연출용) */
	void PlayFadeTransition(FOnFadeEvent OnBlackScreen = FOnFadeEvent(), float InFadeSpeed = 1.f);

// =========================================================================
// 외부 동적 위젯 관리
// =========================================================================
public:
	/** 외부에서 열고 싶은 위젯을 임시로 config data에 등록하고 뷰포트에 보여줍니다. */
	UFUNCTION(BlueprintCallable, Category = "UI|Widgets")
	UECUserWidget* OpenDynamicWidget(FGameplayTag Tag, const FUIConfigWidget& WidgetConfig, AActor* ContextActor = nullptr);
	
	/** 외부에서 임시로 등록한 config data를 제거하고 위젯을 닫습니다. */
	UFUNCTION(BlueprintCallable, Category = "UI|Widgets")
	void CloseDynamicWidget(FGameplayTag Tag);

	/** 특정 태그로 활성화되어 있는 위젯 인스턴스를 반환합니다. */
	UECUserWidget* GetActiveWidget(FGameplayTag Tag) { return ActiveWidgets.FindRef(Tag); }

// =========================================================================
// 내부 위젯 관리
// =========================================================================
private:
	/** 지정된 태그에 해당하는 위젯을 생성(또는 캐시에서 로드)하여 뷰포트에 출력합니다. */
	UECUserWidget* OpenWidget(FGameplayTag Tag, AActor* ContextActor = nullptr);

	/** 지정된 태그에 해당하는 위젯을 뷰포트에서 제거하거나 숨깁니다. */
	void CloseWidget(FGameplayTag Tag);

	/** 위젯의 토글 동작들을 처리합니다.*/
	void ExecuteToggleLogic(FGameplayTag Tag, AActor* ContextActor = nullptr);
	
	/** ESC 입력 시 최상단 위젯을 스택에서 꺼내어 닫습니다. */
	void CloseTopWidget();

	/** 시각적 Z-Order를 최상단으로 끌어올립니다. */
	void BringWidgetToFrontVisually(UECUserWidget* Widget);

	/** 현재 스택 상태에 따라 마우스 커서와 입력 모드를 갱신합니다. */
	void UpdateInputMode();
	
	/** 해당 위젯을 캐싱 정책에 따라 제거하거나 숨깁니다. */
	void DeactivateWidget(FGameplayTag Tag, UECUserWidget* Widget);
	
// =========================================================================
// 위젯 데이터 관리
// =========================================================================
private:
	/** 현재 Map Level에서 사용될 위젯의 정보를 관리합니다. */
	FUIConfigData ConfigData;
	
	/** 활성화된 위젯을 관리합니다. */
	UPROPERTY()
	TMap<FGameplayTag, UECUserWidget*> ActiveWidgets;

	/** 닫힌 위젯의 위치를 기억합니다. */
	UPROPERTY()
	TMap<FGameplayTag, FVector2D> CachedWidgetPositions;

	/** 현재 열려있는 위젯들의 순서를 관리하는 스택 */
	UPROPERTY()
	TArray<FGameplayTag> WidgetStack;

	/** 화면에 띄울 메인 레이아웃 위젯 */
	UPROPERTY()
	UECMainLayoutWidget* MainLayoutWidget;
};