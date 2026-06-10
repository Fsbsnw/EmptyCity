// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "Data/UI/UIConfigDataAsset.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UIManagerSubsystem.generated.h"

struct FInputActionValue;
class UECUserWidget;
class UECMainLayoutWidget;

/**
 * 게임 내에서 자주 여닫는 위젯을 관리하는 서브시스템입니다. 
 */
UCLASS()
class EMPTYCITY_API UUIManagerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	/** 현재 Map Level에서 사용될 위젯의 정보를 저장합니다. */
	void InitializeUIConfig(const FUIConfigData& UIConfig);

	/** 위젯 입력 요청을 태그로 처리합니다. */
	UFUNCTION(BlueprintCallable)	// 블루프린트 테스트용
	void RouteUIInput(FGameplayTag Tag, AActor* ContextActor = nullptr);
	
	/** 위젯 입력 요청을 태그와 입력값으로 처리합니다. */
	void RouteUIInputWithValue(FGameplayTag Tag, const FInputActionValue& Value, AActor* ContextActor = nullptr);
	
	/** 클릭된 위젯에 맞게 Focusing을 업데이트합니다. */
	void UpdateFocusStack(UECUserWidget* FocusedWidget);

// =========================================================================
// 외부에서 동적으로 요청하는 위젯 관리 함수
// =========================================================================
public:
	/** 외부에서 열고 싶은 위젯을 임시로 config data에 등록하고 뷰포트에 보여줍니다. */
	UFUNCTION(BlueprintCallable, Category = "UI|Widgets")
	void OpenDynamicWidget(FGameplayTag Tag, const FUIConfigWidget& WidgetConfig, AActor* ContextActor = nullptr);
	
	/** 외부에서 임시로 등록한 config data를 제거하고 위젯을 닫습니다. */
	UFUNCTION(BlueprintCallable, Category = "UI|Widgets")
	void CloseDynamicWidget(FGameplayTag Tag);

private:
	/** 새로운 위젯을 만들고 뷰포트에 보여줍니다. */
	void OpenWidget(FGameplayTag Tag, AActor* ContextActor = nullptr);
	
	/** 위젯의 Navigation 동작들을 처리합니다. */
	void HandleActionInput(FGameplayTag Tag, const FInputActionValue& Value);
	
	/** 위젯의 토글 동작들을 처리합니다. */
	void HandleToggleInput(FGameplayTag Tag, AActor* ContextActor = nullptr);
	
	/** ESC 입력 시 최상단 위젯을 스택에서 꺼내어 닫습니다. */
	void CloseTopWidget();

	/** IMC 장착/해제를 담당할 헬퍼 함수 */
	void ApplyWidgetInputContext(const FUIConfigWidget& Config, bool bIsOpening);

	/** 시각적 Z-Order를 최상단으로 끌어올립니다. */
	void BringWidgetToFrontVisually(UECUserWidget* Widget);

	/** 현재 스택 상태에 따라 마우스 커서와 입력 모드를 갱신합니다. */
	void UpdateInputMode();

	/** 스택 최상단 위젯에 Focus를 부여합니다. */
	void FocusTopWidget();

	/** 스택 최상단 위젯의 Focus를 회수합니다. */
	void DefocusTopWidget();
	
	/** 해당 위젯을 캐싱 정책에 따라 제거하거나 숨깁니다. */
	void DeactivateWidget(FGameplayTag Tag, UECUserWidget* Widget);
	
// =========================================================================
// Widget Data 관리
// =========================================================================
private:
	/** 현재 Map Level에서 사용될 위젯의 정보를 관리합니다. */
	FUIConfigData ConfigData;
	
	/** 활성화된 위젯을 관리합니다. */
	UPROPERTY()
	TMap<FGameplayTag, UECUserWidget*> ActiveWidgets;

	/** 현재 열려있는 위젯들의 순서를 관리하는 스택 */
	UPROPERTY()
	TArray<FGameplayTag> WidgetStack;

	/** 화면에 띄울 메인 레이아웃 위젯 */
	UPROPERTY()
	UECMainLayoutWidget* MainLayoutWidget;
};
