// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "View/MVVMView.h"
#include "MVVMViewModelBase.h"
#include "ECUserWidget.generated.h"

/**
 * @brief 프로젝트의 기본 UI를 구성하는 커스텀 User Widget 베이스 클래스입니다.
 * MVVM 패턴 연동, 위젯 창 드래그, 포커스 시각 효과 처리, 
 * 그리고 부모-자식 간의 계층 구조 및 닫기(ESC) 기능을 제공합니다.
 */
UCLASS()
class EMPTYCITY_API UECUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
// ============================================================================
// 생성자 / 소멸자 관리
// ============================================================================
	
public:	
	/** 위젯이 화면에 나타날 때마다 UI Manager가 호출해 줄 함수 */
	virtual void OnWidgetOpened();

	/** 위젯이 화면에서 사라질 때마다 UI Manager가 호출해 줄 함수 */
	virtual void OnWidgetClosed();

// ============================================================================
// MVVM 뷰모델 관리
// ============================================================================
	
public:
	/** 외부(Subsystem이나 부모)에서 이 위젯에 뷰모델을 주입할 때 사용하는 범용 함수 */
	UFUNCTION(BlueprintCallable, Category = "MVVM")
	virtual void InjectViewModel(UMVVMViewModelBase* TargetViewModel);

	/** 런타임 MVVM View Extension 쉽게 가져오기 위한 헬퍼 함수 */
	UFUNCTION(BlueprintPure, Category = "MVVM")
	UMVVMView* GetMVVMView() const;

	/** 위젯에 등록된 뷰모델 하나를 가져오는 함수 */
	template<typename T>
	T* GetViewModel() const
	{
		if (const TObjectPtr<UMVVMViewModelBase>* Found = CachedViewModels.Find(T::StaticClass()))
		{
			return Cast<T>(*Found);
		}
		return nullptr;
	}

private:
	UPROPERTY()
	TMap<TSubclassOf<UMVVMViewModelBase>, TObjectPtr<UMVVMViewModelBase>> CachedViewModels;

// ============================================================================
// 블루프린트 위젯 창 관련 함수
// ============================================================================
	
protected:	
	/**
	 * 위젯 창을 Drag 하기 위한 함수들입니다.
	 */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Focus를 얻을 때 실행되는 함수입니다. */
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	/** Focus를 잃을 때 실행되는 함수입니다. */
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

// ============================================================================
// 블루프린트 위젯 창 관련 데이터
// ============================================================================
	
private:
	/** Drag 상태 추적용 변수 */
	bool bIsDragging = false;
	
	/** 드래그가 시작된 마우스의 화면 좌표입니다. */
	FVector2D DragStartMousePosition;
    
	/** 드래그가 시작될 때 위젯의 로컬 렌더 트랜스폼 좌표입니다. */
	FVector2D DragStartWidgetTranslation;
    
	/** Drag 이동이 가능한지 여부를 결정합니다. */
	UPROPERTY(EditAnywhere, Category = "Drag")
	bool bIsDraggable = false;

// ============================================================================
// 위젯 계층 관리
// ============================================================================
public:
	/** 자식을 등록하는 함수 */
	UFUNCTION(BlueprintCallable, Category = "Children")
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

private:	
	UPROPERTY()
	TObjectPtr<UECUserWidget> ParentWidget = nullptr;
	UPROPERTY()
	TArray<UECUserWidget*> ChildWidgets;
	
// ============================================================================
// 블루프린트 호출 함수
// ============================================================================
public:
	/** ViewModel이 성공적으로 주입되었을 때 블루프린트에서 추가 로직을 처리하는 이벤트입니다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "MVVM", meta = (DisplayName = "On View Model Set"))
	void OnViewModelSet();
	
	/** 위젯이 최상단으로 올라와 Focus를 얻었을 때 실행되는 시각 효과(밝아지는 애니메이션 실행) 이벤트입니다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Focus")
	void OnFocusGainedAnim();

	/** 다른 창에 가려지거나 Focus를 잃었을 때 실행되는 시각 효과(어두워지는 애니메이션 실행) 이벤트입니다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Focus")
	void OnFocusLostAnim();
};