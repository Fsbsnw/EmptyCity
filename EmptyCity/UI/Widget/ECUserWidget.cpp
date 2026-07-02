// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/ECUserWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "UI/Subsystem/UIManagerSubsystem.h"

void UECUserWidget::OnWidgetOpened()
{
}

void UECUserWidget::OnWidgetClosed()
{
}

void UECUserWidget::InjectViewModel(UMVVMViewModelBase* TargetViewModel)
{
	if (!IsValid(TargetViewModel))
	{
		return;
	}

	CachedViewModels.Add(TargetViewModel->GetClass(), TargetViewModel);
	
	if (UMVVMView* MVVMView = GetMVVMView())
	{
		// MVVM 익스텐션 시스템에 뷰모델 인스턴스 등록
		MVVMView->SetViewModelByClass(TargetViewModel);
	}
}

UMVVMView* UECUserWidget::GetMVVMView() const
{
	// 위젯에 부착된 MVVMView 익스텐션을 안전하게 찾아 반환합니다.
	return GetExtension<UMVVMView>();
}

void UECUserWidget::AddChildWidget(UECUserWidget* Child)
{
	if (Child)
	{
		// 자식 위젯의 부모를 this로 설정합니다.
		Child->ParentWidget = this;
		// 자식 위젯 관리를 위해 등록합니다.
		ChildWidgets.Add(Child);
	}
}

void UECUserWidget::ClearAllChildren()
{
	for (UECUserWidget* Child : ChildWidgets)
	{
		if (Child)
		{
			// 자식들이 갖고 있는 다른 위젯들을 재귀적으로 제거합니다. 
			Child->ClearAllChildren();
			Child->RemoveFromParent();
		}
	}
	ChildWidgets.Empty();
}

void UECUserWidget::CloseWidgetAndChildren()
{
	ClearAllChildren();
	RemoveFromParent();
}

void UECUserWidget::PushChild(UECUserWidget* FocusedChild)
{
	if (!FocusedChild)
	{
		return;
	}

	// 기존 스택에 이미 있으면 잠시 빼냅니다.
	ChildWidgets.Remove(FocusedChild);

	// 스택의 가장 최상단으로 밀어 넣습니다.
	ChildWidgets.Push(FocusedChild);
}

bool UECUserWidget::HandleCloseRequest()
{
	// 스택에 쌓인 순서대로 검사합니다
	while (ChildWidgets.Num() > 0)
	{
		// 스택의 최상단 자식을 꺼냅니다.
		UECUserWidget* TopChild = ChildWidgets.Last();

		// 유효성 검사
		if (TopChild && TopChild->IsVisible())
		{
			// 가장 최근 자식을 닫습니다.
			TopChild->RemoveFromParent();
            
			// 스택에서 제거합니다.
			ChildWidgets.Pop();

			// UIManager에게 결과 전달
			return true; 
		}

		// 유효하지 않은 위젯이면 스택에서만 빼고 다음 위젯으로 넘어갑니다.
		ChildWidgets.Pop();
	}

	// 스택이 완전히 비어있다면, 더 이상 닫을 자식이 없으므로 나를 닫아달라고 전달
	return false;
}

FReply UECUserWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 드래그가 켜져 있고, 마우스 왼쪽 버튼을 눌렀다면 드래그 시작!
	if (bIsDraggable && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;
		DragStartMousePosition = InMouseEvent.GetScreenSpacePosition();
		DragStartWidgetTranslation = GetRenderTransform().Translation;

		// 마우스가 위젯을 계속 캡처합니다.
		return FReply::Handled().CaptureMouse(TakeWidget());
	}
	return FReply::Handled();
}

FReply UECUserWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDragging)
	{
		// 1. 마우스가 얼마나 이동했는지 계산
		FVector2D MouseDelta = InMouseEvent.GetScreenSpacePosition() - DragStartMousePosition;

		// 2. 현재 화면의 DPI 스케일로 나눠주어야 마우스와 1:1로 딱 붙어서 이동합니다.
		float DPIScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
		FVector2D NewTranslation = DragStartWidgetTranslation + (MouseDelta / DPIScale);

		// 3. 위젯의 렌더링 위치 업데이트
		SetRenderTranslation(NewTranslation);

		return FReply::Handled();
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UECUserWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDragging && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = false;
        
		// 마우스 캡처를 풀어줍니다.
		return FReply::Handled().ReleaseMouseCapture();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UECUserWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	// 최상위(Root) 위젯을 찾기 위한 포인터 추적 시작
	UECUserWidget* CurrentWidget = this;

	// 부모가 존재하는 동안(자식 위젯인 동안) 계속 위로 올라감
	while (CurrentWidget->ParentWidget)
	{
		// 해당 부모의 로컬 팝업 스택 맨 위로 현재 위젯을 올림
		CurrentWidget->ParentWidget->PushChild(CurrentWidget);
       
		// 한 단계 위 부모로 포인터 이동
		CurrentWidget = CurrentWidget->ParentWidget;
	}

	// While 문이 끝나면 CurrentWidget은 매니저에 등록된 최종 Root 위젯이 됩니다.
	if (UUIManagerSubsystem* UIManager = GetOwningLocalPlayer()->GetSubsystem<UUIManagerSubsystem>())
	{
		UIManager->UpdateFocusStack(CurrentWidget);
	}

	// Focus 획득 애니메이션 실행
	OnFocusGainedAnim();
	
	Super::NativeOnAddedToFocusPath(InFocusEvent);
}

void UECUserWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	// Focus 잃는 애니메이션 실행
	OnFocusLostAnim();
	
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
}