// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ECUserWidget.generated.h"

class UMVVMView;
class UMVVMViewModelBase;

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECUserWidget : public UUserWidget
{
	GENERATED_BODY()

// ============================================================================
// MVVM 뷰모델 핵심 제어 함수
// ============================================================================
	
public:
	/** 외부(Subsystem이나 부모)에서 이 위젯에 뷰모델을 주입할 때 사용하는 범용 함수 */
	UFUNCTION(BlueprintCallable, Category = "MVVM")
	void InjectViewModel(UMVVMViewModelBase* TargetViewModel);

	/** 런타임 MVVM View Extension 쉽게 가져오기 위한 헬퍼 함수 */
	UFUNCTION(BlueprintPure, Category = "MVVM")
	UMVVMView* GetMVVMView() const;

// ============================================================================
// 위젯 계층 및 Focus 관리 함수
// ============================================================================

protected:
	/** 이 위젯이 클릭 되었을 때 Focus를 획득하는 함수를 호출하기 위해 오버라이드를 합니다. */
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
public:
	/** 자식을 등록하는 함수 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void AddChildWidget(UECUserWidget* Child);

	/** 자신과 모든 자식들을 제거하는 함수 */
	void CloseWidgetAndChildren();
	/** 모든 자식들을 제거하는 함수 */
	void ClearAllChildren();
	/** 자식 위젯이 켜지거나 클릭되어 포커스를 잡았을 때 부모에게 순서 재정렬을 요청하는 함수 */
	void PushChild(UECUserWidget* FocusedChild);

	/** UIManager로부터 ESC 요청을 받았을 때 호출되는 함수입니다.
	 * 내부 Stack에 등록된 자식 위젯을 처리했다면 true,
	 * 더 이상 닫을 자식이 없어서 부모가 닫혀야 한다면 false를 반환합니다.
	 */
	virtual bool HandleCloseRequest(); 

	/** 이 위젯이 클릭되었을 때 자동으로 Focus를 얻고 맨 앞으로 올릴 것인가? 
	 * 일반적인 창들은 true, HUD나 툴팁, 데미지 숫자는 false로 끕니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Focus Settings")
	bool bAutoFocusWithClick = false;

private:	
	UPROPERTY()
	TObjectPtr<UECUserWidget> ParentWidget = nullptr;
	UPROPERTY()
	TArray<UECUserWidget*> ChildWidgets;
	
// ============================================================================
// 블루프린트 호출 함수
// ============================================================================
public:
	/** ViewModel이 주입되었을 때 실행될 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "MVVM", meta = (DisplayName = "On View Model Set"))
	void OnViewModelSet();
	
	/** 맨 앞으로 나와서 Focus를 얻었을 때 (밝아지는 애니메이션 실행) */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI | Focus")
	void OnFocusGainedAnim();

	/** 다른 창에 가려져 Focus를 잃었을 때 (어두워지는 애니메이션 실행) */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI | Focus")
	void OnFocusLostAnim();
};