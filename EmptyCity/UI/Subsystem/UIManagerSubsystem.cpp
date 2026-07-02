// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Subsystem/UIManagerSubsystem.h"

#include "ECGameplayTags.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Data/UI/UIConfigDataAsset.h"
#include "Framework/Application/NavigationConfig.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "UI/ViewModel/ECViewModelFactoryLibrary.h"
#include "UI/Widget/ECMainLayoutWidget.h"
#include "UI/Widget/ECUserWidget.h"
#include "UI/Widget/FadeScreen/ECFadeScreenWidget.h"

// =========================================================================
// 화면 전환 연출
// =========================================================================
void UUIManagerSubsystem::PlayFadeOut(FOnFadeEvent OnFinishedCallback, float InFadeSpeed)
{
	FGameplayTag FadeOutTag = ECGameplayTags::InputTag_UI_FadeScreen_Out;
    
	if (UECUserWidget* Widget = OpenWidget(FadeOutTag))
	{
		if (UECFadeScreenWidget* FadeOut = Cast<UECFadeScreenWidget>(Widget))
		{
			FadeOut->PlayTransition(InFadeSpeed);
			// 람다를 통해 애니메이션이 끝나면 매니저가 스스로 제거합니다.
			FadeOut->OnFadeFinishedDelegate.AddWeakLambda(this, [this, FadeOutTag, OnFinishedCallback]()
			{
				this->CloseWidget(FadeOutTag);
				OnFinishedCallback.ExecuteIfBound();
			});
		}
	}
}

void UUIManagerSubsystem::PlayFadeIn(FOnFadeEvent OnFinishedCallback, float InFadeSpeed)
{
	FGameplayTag FadeInTag = ECGameplayTags::InputTag_UI_FadeScreen_In;
    
	if (UECUserWidget* Widget = OpenWidget(FadeInTag))
	{
		if (UECFadeScreenWidget* FadeIn = Cast<UECFadeScreenWidget>(Widget))
		{
			FadeIn->PlayTransition(InFadeSpeed);
			// 람다를 통해 애니메이션이 끝나면 매니저가 스스로 제거합니다.
			FadeIn->OnFadeFinishedDelegate.AddWeakLambda(this, [this, FadeInTag, OnFinishedCallback]()
			{
				this->CloseWidget(FadeInTag);
				OnFinishedCallback.ExecuteIfBound();
			});
		}
	}
}

void UUIManagerSubsystem::PlayFadeTransition(FOnFadeEvent OnBlackScreen, float InFadeSpeed)
{
	// 페이드 아웃과 페이드 인을 하나의 시퀀스로 실행합니다.
	PlayFadeOut(FOnFadeEvent::CreateWeakLambda(this,
		[this, OnBlackScreen, InFadeSpeed]()
		{
			// 1. 화면 까매짐 -> 원하는 외부 로직(지도 띄우기 등) 실행
			OnBlackScreen.ExecuteIfBound();

			// 2. 다 쓴 페이드 아웃 위젯 파괴
			CloseWidget(ECGameplayTags::InputTag_UI_FadeScreen_Out);

			// 3. 페이드 인 실행 (끝나면 알아서 파괴됨)
			PlayFadeIn(FOnFadeEvent(), InFadeSpeed);
		}),
		InFadeSpeed
	);
}

// =========================================================================
// 위젯 관리
// =========================================================================
void UUIManagerSubsystem::InitializeUIConfig(const FUIConfigData& UIConfig)
{
	this->ConfigData = UIConfig;
	ActiveWidgets.Empty();
	WidgetStack.Empty();

	// 게임 시작 시 전역 네비게이션 설정을 세팅
	if (FSlateApplication::IsInitialized())
	{
		TSharedRef<FNavigationConfig> NavigationConfig = FSlateApplication::Get().GetNavigationConfig();
		NavigationConfig->bTabNavigation = false;
		NavigationConfig->bKeyNavigation = false;     // 필요에 따라 선택 (방향키 네비게이션 차단)
		NavigationConfig->bAnalogNavigation = false;  // 필요에 따라 선택 (패드 아날로그 스틱 차단)
	}

	// 데이터 에셋에 등록된 MainLayout 클래스가 있다면 생성
	if (UIConfig.MainLayoutClass)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		MainLayoutWidget = CreateWidget<UECMainLayoutWidget>(PC, UIConfig.MainLayoutClass);

		if (MainLayoutWidget)
		{
			// 이 시스템에서 AddToViewport는 여기서 한 번만 불립니다.
			MainLayoutWidget->AddToViewport(); 
		}
	}
}

UECUserWidget* UUIManagerSubsystem::OpenDynamicWidget(FGameplayTag Tag, const FUIConfigWidget& WidgetConfig, AActor* ContextActor)
{
	// 1. 전달받은 임시 설정을 매니저의 ConfigData에 등록 (기존에 같은 태그가 있다면 덮어씌움)
	ConfigData.WidgetMap.Add(Tag, WidgetConfig);

	// 2. 동적 위젯은 무조건 닫힐 때 파괴(Destroy)되도록 정책 강제 고정!
	ConfigData.WidgetMap[Tag].CachePolicy = EWidgetCachePolicy::DestroyOnClose;

	// 3. 임시 등록 완료 후 위젯 토글 요청 
	RequestToggleInput(Tag, ContextActor);

	return ActiveWidgets.Contains(Tag) ? ActiveWidgets[Tag] : nullptr;
}

void UUIManagerSubsystem::CloseDynamicWidget(FGameplayTag Tag)
{
	// 현재 열려있는 위젯인지 확인
	if (UECUserWidget** FoundWidget = ActiveWidgets.Find(Tag))
	{
		UECUserWidget* Widget = *FoundWidget;

		if (Widget->IsVisible())
		{
			// 스택에서 제거하고 권한 뺏기
			WidgetStack.Remove(Tag);

			// 파괴 또는 숨김
			DeactivateWidget(Tag, Widget);
			ConfigData.WidgetMap.Remove(Tag);

			// 입력 모드 갱신
			UpdateInputMode();
		}
	}
}

void UUIManagerSubsystem::RequestToggleInput(FGameplayTag Tag, AActor* ContextActor)
{
	// 위젯 닫기 입력 처리
	if (Tag.MatchesTagExact(ECGameplayTags::InputTag_UI_Navigation_Escape))
	{
		CloseTopWidget();
		return;
	}
	
	// 1. 데이터 에셋에 해당 태그가 있는지, 트랜지션 설정이 켜져 있는지 확인
	bool bUseTransition = false;
	if (ConfigData.WidgetMap.Contains(Tag))
	{
		bUseTransition = (ConfigData.WidgetMap[Tag].TransitionType == EWidgetTransitionType::FadeTransition);
	}

	// 2. 트랜지션이 켜져 있다면: 페이드 아웃 연출 이후에 ToggleLogic 실행
	if (bUseTransition)
	{
		PlayFadeTransition(FOnFadeEvent::CreateWeakLambda(this, [this, Tag, ContextActor]()
		{
			this->ExecuteToggleLogic(Tag, ContextActor);
		}));
	}
	// 3. 트랜지션이 없다면 (일반 인벤토리 등): 기다리지 않고 즉시 ToggleLogic 실행
	else
	{
		ExecuteToggleLogic(Tag, ContextActor);
	}
}

void UUIManagerSubsystem::ExecuteToggleLogic(FGameplayTag Tag, AActor* ContextActor)
{
	// 1. 이미 활성화된 위젯인 경우
	if (UECUserWidget** FoundWidget = ActiveWidgets.Find(Tag))
	{
		UECUserWidget* Widget = *FoundWidget;
		
		// 2. 이미 위젯이 뷰포트에 보이고 있는 경우 -> 닫기(비활성화)
		if (Widget->IsVisible())
		{
			// 스택 중간이나 최상단에 있던 해당 태그를 제거
			WidgetStack.Remove(Tag);
			
			// 캐싱 정책에 따라 위젯 숨기기/삭제
			DeactivateWidget(Tag, Widget);
		}
		// 3. 위젯이 캐싱되어 숨겨진 상태인 경우 -> 다시 보여주기
		else
		{
			// 현재 위젯 보여주고 Z-Order 높이기
			BringWidgetToFrontVisually(Widget);

			// 캐싱된 위젯이 다시 열렸음을 알림
			Widget->OnWidgetOpened();

			// 최상단 위젯에 등록
			WidgetStack.Push(Tag);
		}
		// 입력 모드를 업데이트 합니다.
		UpdateInputMode();
	}
	// 처음 위젯을 호출하는 경우
	else
	{
		// 일반 위젯(인벤토리 등)은 즉시 열기
		UECUserWidget* NewWidget = OpenWidget(Tag, ContextActor);
       
		if (NewWidget)
		{
			// 새로운 위젯이 정상적으로 열렸음을 알림
			NewWidget->OnWidgetOpened();
		}
	}
}

UECUserWidget* UUIManagerSubsystem::OpenWidget(FGameplayTag Tag, AActor* ContextActor)
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return nullptr;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->IsLocalController()) return nullptr;

	if (!ConfigData.WidgetMap.Contains(Tag)) return nullptr;

	// 위젯 생성
	UECUserWidget* Widget = CreateWidget<UECUserWidget>(PC, ConfigData.WidgetMap[Tag].WidgetClass);
	if (Widget == nullptr) return nullptr;

	// 필요한 모든 뷰모델을 주입 
	for (TSubclassOf<UECViewModelBase> SourceVM : ConfigData.WidgetMap[Tag].ViewModels)
	{
		// ContextActor를 바탕으로 필요한 ViewModel을 생성 및 바인딩
		UECViewModelBase* NewViewModel = UECViewModelFactoryLibrary::CreateViewModel(SourceVM, ContextActor, Widget);

		if (NewViewModel)
		{
			// ViewModel 주입
			Widget->InjectViewModel(NewViewModel);

			// ViewModel 초기값 적용
			NewViewModel->BroadcastInitialValues();
		}
	}

	// 모든 뷰모델 주입을 완료한 후에 호출되는 이벤트입니다.
	Widget->OnViewModelSet();

	// 3층: System 레이어 (연출 전용: 페이드, 꿈 UI 등)
	if (ConfigData.WidgetMap[Tag].LayerType == EWidgetLayerType::System)
	{
		Widget->AddToViewport(100); // 메인 레이아웃의 숨김 여부와 무관하게 무조건 화면 최상단에 덮음
        
		// 나중에 지울 수 있게 ActiveWidgets에만 등록하고, 
		// 포커스/스택/입력 관리 로직은 건너뛰고 즉시 종료합니다!
		ActiveWidgets.Add(Tag, Widget);
		return Widget; 
	}

	// ==========================================================
	// 레이어 분리 로직 (HUD vs Window)
	// ==========================================================
	if (MainLayoutWidget)
	{
		if (ConfigData.WidgetMap[Tag].LayerType == EWidgetLayerType::HUD)
		{
			// 1층: HUD 레이어 부착 (꽉 채우기)
			UOverlaySlot* HUDSlot = MainLayoutWidget->Layer_HUD->AddChildToOverlay(Widget);
			if (HUDSlot)
			{
				HUDSlot->SetHorizontalAlignment(HAlign_Fill);
				HUDSlot->SetVerticalAlignment(VAlign_Fill);
			}
			ActiveWidgets.Add(Tag, Widget);
			return Widget; // HUD는 스택/입력 관리를 안 하므로 여기서 즉시 종료
		}
		else
		{
			// 2층: 일반 창 레이어 부착
			UCanvasPanelSlot* WindowSlot = MainLayoutWidget->Layer_Window->AddChildToCanvas(Widget);
			if (WindowSlot)
			{
				WindowSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f)); 
				WindowSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 0.f));
			}

			// 이전에 저장된 위치가 있다면 위젯에 덮어씌워 줍니다.
			if (CachedWidgetPositions.Contains(Tag))
			{
				Widget->SetRenderTranslation(CachedWidgetPositions[Tag]);
			}
			
			// 새로 생성된 창을 맨 앞에 표시합니다.
			BringWidgetToFrontVisually(Widget);
		}
	}
	else
	{
		// 최악의 경우 대비 (MainLayout 누락 시 뷰포트에 띄움)
		Widget->AddToViewport();
	}

	// 새로 만든 위젯 정보를 등록합니다.
	ActiveWidgets.Add(Tag, Widget);
	WidgetStack.Push(Tag);

	// 입력 모드를 업데이트 합니다.
	UpdateInputMode();

	return Widget;
}

void UUIManagerSubsystem::CloseWidget(FGameplayTag Tag)
{
	// 현재 열려있는 위젯인지 확인
	if (UECUserWidget** FoundWidget = ActiveWidgets.Find(Tag))
	{
		UECUserWidget* Widget = *FoundWidget;

		if (Widget->IsVisible())
		{
			// 스택에서 제거하고 권한 뺏기
			WidgetStack.Remove(Tag);

			// 파괴 또는 숨김 (ConfigData.WidgetMap.Remove(Tag)는 하지 않습니다!)
			DeactivateWidget(Tag, Widget);

			// 입력 모드 갱신
			UpdateInputMode();
		}
	}
}

void UUIManagerSubsystem::CloseTopWidget()
{
	if (WidgetStack.IsEmpty())
	{
		return;
	}

	// 스택의 최상단 요소를 꺼냅니다.
	FGameplayTag TopTag = WidgetStack.Last();

	if (UECUserWidget** FoundWidget = ActiveWidgets.Find(TopTag))
	{
		UECUserWidget* Widget = *FoundWidget;

		// 자식 팝업 위젯을 닫았으면 함수 종료
		if (Widget->HandleCloseRequest())
		{
			return;
		}
        
		// 위젯을 비활성화
		DeactivateWidget(TopTag, Widget);
		WidgetStack.Remove(TopTag);
	}
	
	UpdateInputMode();
}

void UUIManagerSubsystem::BringWidgetToFrontVisually(UECUserWidget* Widget)
{
	if (Widget)
	{
		Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
		{
			static int32 GlobalZOrder = 100;
			CanvasSlot->SetZOrder(++GlobalZOrder);
		}
	}
}

void UUIManagerSubsystem::UpdateInputMode()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	// 스택에 열려있는 위젯이 하나라도 있다면
	if (!WidgetStack.IsEmpty())
	{
		FGameplayTag TopTag = WidgetStack.Last();
        
		// 최상단 위젯의 Config 데이터를 가져와서 마우스 커서 표시 여부를 결정합니다.
		if (ConfigData.WidgetMap.Contains(TopTag))
		{
			PC->bShowMouseCursor = ConfigData.WidgetMap[TopTag].bShowMouseCursor;
		}

		// 입력 모드 설정 (UI와 게임 입력을 모두 받음)
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        
		// 최상단 위젯에 포커스 부여
		if (UECUserWidget** TopWidget = ActiveWidgets.Find(TopTag))
		{
			InputMode.SetWidgetToFocus((*TopWidget)->TakeWidget());
		}

		PC->SetInputMode(InputMode);
	}
	// 스택이 비어있다면 (모든 UI가 닫힘)
	else
	{
		// 기본 상태(마우스 숨김)로 복귀
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void UUIManagerSubsystem::DeactivateWidget(FGameplayTag Tag, UECUserWidget* Widget)
{
	if (!Widget) return;

	CachedWidgetPositions.Add(Tag, Widget->GetRenderTransform().Translation);
	
	Widget->OnWidgetClosed();

	if (ConfigData.WidgetMap[Tag].CachePolicy == EWidgetCachePolicy::DestroyOnClose)
	{
		Widget->CloseWidgetAndChildren();
		ActiveWidgets.Remove(Tag);
	}
	else
	{
		Widget->SetVisibility(ESlateVisibility::Collapsed);
		Widget->ClearAllChildren();
	}
}

void UUIManagerSubsystem::UpdateFocusStack(UECUserWidget* FocusedWidget)
{
	if (!FocusedWidget)
	{
		return;
	}

	FGameplayTag FoundTag = FGameplayTag::EmptyTag;

	// ActiveWidgets 맵을 순회하며 이 포인터의 주인이 어떤 태그인지 찾습니다.
	for (const auto& Pair : ActiveWidgets)
	{
		if (Pair.Value == FocusedWidget)
		{
			FoundTag = Pair.Key;
			break;
		}
	}

	// 정식 등록된 Root 창이 맞고, 스택에 존재한다면 재정렬 실행
	if (FoundTag.IsValid() && WidgetStack.Contains(FoundTag))
	{
		// 이미 최상단 위젯이라면 굳이 재정렬 할 필요가 없습니다.
		if (WidgetStack.Last() == FoundTag)
		{
			return;
		}

		// 선택된 위젯을 스택 최상단으로 이동
		WidgetStack.Remove(FoundTag);
		WidgetStack.Push(FoundTag);

		// 위젯의 Z-Order 갱신
		BringWidgetToFrontVisually(FocusedWidget);

		// 마우스 포커스 갱신 (선택된 위젯에 입력 포커스 강제)
		UpdateInputMode();
	}
}