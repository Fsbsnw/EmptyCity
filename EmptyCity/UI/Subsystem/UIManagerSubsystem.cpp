// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Subsystem/UIManagerSubsystem.h"

#include "ECGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "Character/Player/Controller/ECPlayerController.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Data/UI/UIConfigDataAsset.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "UI/ViewModel/ECViewModelFactoryLibrary.h"
#include "UI/Widget/ECMainLayoutWidget.h"
#include "UI/Widget/ECUserWidget.h"
#include "UI/Widget/Interface/NavigableWidgetInterface.h"


void UUIManagerSubsystem::InitializeUIConfig(const FUIConfigData& UIConfig)
{
	this->ConfigData = UIConfig;
	ActiveWidgets.Empty();
	WidgetStack.Empty();

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

void UUIManagerSubsystem::OpenDynamicWidget(FGameplayTag Tag, const FUIConfigWidget& WidgetConfig, AActor* ContextActor)
{
	// 1. 전달받은 임시 설정을 매니저의 ConfigData에 등록 (기존에 같은 태그가 있다면 덮어씌움)
	ConfigData.WidgetMap.Add(Tag, WidgetConfig);

	// 2. 동적 위젯은 무조건 닫힐 때 파괴(Destroy)되도록 정책 강제 고정!
	ConfigData.WidgetMap[Tag].CachePolicy = EWidgetCachePolicy::DestroyOnClose;

	// 3. 임시 등록 완료 후 위젯 토글 요청 
	HandleToggleInput(Tag, ContextActor);
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
			ApplyWidgetInputContext(ConfigData.WidgetMap[Tag], false);

			// 파괴 또는 숨김
			DeactivateWidget(Tag, Widget);
			ConfigData.WidgetMap.Remove(Tag);
			
			// 밑에 깔려있던 위젯 포커스 살리기
			FocusTopWidget();

			// 입력 모드 갱신
			UpdateInputMode();
		}
	}
}

void UUIManagerSubsystem::RouteUIInput(FGameplayTag Tag, AActor* ContextActor)
{
	HandleToggleInput(Tag, ContextActor);
}

void UUIManagerSubsystem::RouteUIInputWithValue(FGameplayTag Tag, const FInputActionValue& Value, AActor* ContextActor)
{
	// 1. 조작(Action) 입력 태그인 경우 (WASD, Enter, ESC 등)
	if (Tag.MatchesTag(ECGameplayTags::InputTag_UI_Navigation))
	{
		HandleActionInput(Tag, Value);	
	}
	// 2. 위젯 토글(Toggle) 요청인 경우 (인벤토리 키, 맵 키 등 단축키 입력)
	else
	{
		HandleToggleInput(Tag, ContextActor);
	}	
}

void UUIManagerSubsystem::HandleActionInput(FGameplayTag Tag, const FInputActionValue& Value)
{
	if (WidgetStack.IsEmpty())
	{
		return;
	}

	// 스택의 가장 마지막(최상단) 위젯 태그를 가져옵니다.
	FGameplayTag TopTag = WidgetStack.Last();
	UECUserWidget* TopWidget = ActiveWidgets[TopTag];

	// Escape 입력이 들어온 경우 즉시 최상단 위젯을 닫습니다.
	if (Tag.MatchesTagExact(ECGameplayTags::InputTag_UI_Navigation_Escape))
	{
		CloseTopWidget();
		return;
	}

	// 네비게이션 인터페이스 미구현 시 조작 불가
	if (!TopWidget->Implements<UNavigableWidgetInterface>()) return;
	
	// Direction 입력: 위젯 방향 이동
	if (Tag.MatchesTagExact(ECGameplayTags::InputTag_UI_Navigation_Direction))
	{
		FVector2D MoveVec = Value.Get<FVector2D>();

		// 들어온 입력을 UI에서의 방향으로 변환합니다.
		EWidgetNavDirection Direction = (FMath::Abs(MoveVec.X) > FMath::Abs(MoveVec.Y))
            ? ((MoveVec.X > 0.f) ? EWidgetNavDirection::Right : EWidgetNavDirection::Left)
            : ((MoveVec.Y > 0.f) ? EWidgetNavDirection::Up : EWidgetNavDirection::Down);
		
		INavigableWidgetInterface::Execute_ReceiveDirectionInput(TopWidget, Direction);
		return;
	}
	
	// Confirm 입력: 위젯 확인 동작
	if (Tag.MatchesTagExact(ECGameplayTags::InputTag_UI_Navigation_Confirm))
	{
		INavigableWidgetInterface::Execute_ReceiveConfirmInput(TopWidget);
		return;
	}
}

void UUIManagerSubsystem::HandleToggleInput(FGameplayTag Tag, AActor* ContextActor)
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
			ApplyWidgetInputContext(ConfigData.WidgetMap[Tag], false);

			// 캐싱 정책에 따라 위젯 숨기기/삭제
			DeactivateWidget(Tag, Widget);
		}
		// 3. 위젯이 캐싱되어 숨겨진 상태인 경우 -> 다시 보여주기
		else
		{
			// 기존 최상단 위젯 숨기기
			DefocusTopWidget();

			// 현재 위젯 보여주고 Z-Order 높이기
			Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			BringWidgetToFrontVisually(Widget);

			// 최상단 위젯에 등록
			WidgetStack.Push(Tag);
		}
		// 변경된 최상단 위젯에 Focus를 부여합니다.
		FocusTopWidget();
		// 입력 모드를 업데이트 합니다.
		UpdateInputMode();
	}
	// 처음 위젯을 호출하는 경우
	else
	{
		OpenWidget(Tag, ContextActor);
	}
}

void UUIManagerSubsystem::OpenWidget(FGameplayTag Tag, AActor* ContextActor)
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->IsLocalController()) return;

	// 위젯 생성
	UECUserWidget* Widget = CreateWidget<UECUserWidget>(PC, ConfigData.WidgetMap[Tag].WidgetClass);
	if (Widget == nullptr) return;

	// 필요한 모든 뷰모델을 주입 
	for (TSubclassOf<UECViewModelBase> SourceVM : ConfigData.WidgetMap[Tag].ViewModels)
	{
		// ContextActor를 바탕으로 필요한 ViewModel을 생성
		UECViewModelBase* NewViewModel = UECViewModelFactoryLibrary::CreateViewModel(SourceVM, ContextActor, Widget);

		if (NewViewModel)
		{
			// ViewModel 주입
			Widget->InjectViewModel(NewViewModel);
		}
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
			return; // HUD는 스택/입력 관리를 안 하므로 여기서 즉시 종료
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
			
			// 새로 생성된 창을 맨 앞에 표시합니다.
			BringWidgetToFrontVisually(Widget);
		}
	}
	else
	{
		// 최악의 경우 대비 (MainLayout 누락 시 뷰포트에 띄움)
		Widget->AddToViewport();
	}

	// 기존의 최상단 위젯의 Focus를 해제합니다.
	DefocusTopWidget();

	// 새로 만든 위젯 정보를 등록합니다.
	ActiveWidgets.Add(Tag, Widget);
	WidgetStack.Push(Tag);

	// 새로 추가된 위젯에 Focus를 부여합니다.
	FocusTopWidget();

	// 입력 모드를 업데이트 합니다.
	UpdateInputMode();
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
        
		// 닫히는 위젯의 IMC 해제
		ApplyWidgetInputContext(ConfigData.WidgetMap[TopTag], false);
		// 위젯을 비활성화
		DeactivateWidget(TopTag, Widget);
		WidgetStack.Remove(TopTag);
	}

	// 새롭게 최상단으로 올라온 위젯에 Focus를 부여합니다.
	FocusTopWidget();
	
	UpdateInputMode();
}

void UUIManagerSubsystem::ApplyWidgetInputContext(const FUIConfigWidget& Config, bool bIsOpening)
{
	// 특별히 덮어씌울 IMC가 없다면 무시합니다 (예: 인벤토리는 IMC 교체 불필요, 맵만 필요 등)
	if (!Config.OverrideIMC) return;

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (bIsOpening)
			{
				Subsystem->AddMappingContext(Config.OverrideIMC, Config.IMCPriority);
			}
			else
			{
				Subsystem->RemoveMappingContext(Config.OverrideIMC);
			}
		}
	}
}

void UUIManagerSubsystem::BringWidgetToFrontVisually(UECUserWidget* Widget)
{
	if (Widget)
	{
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

void UUIManagerSubsystem::FocusTopWidget()
{
	if (WidgetStack.IsEmpty())
	{
		return;
	}

	FGameplayTag TopTag = WidgetStack.Last();
	ApplyWidgetInputContext(ConfigData.WidgetMap[TopTag], true);

	// 새롭게 최상단으로 올라온 위젯이 밝아집니다.
	if (UECUserWidget** NewTop = ActiveWidgets.Find(TopTag))
	{
		(*NewTop)->OnFocusGainedAnim();
	}
}

void UUIManagerSubsystem::DeactivateWidget(FGameplayTag Tag, UECUserWidget* Widget)
{
	if (!Widget) return;

	if (ConfigData.WidgetMap[Tag].CachePolicy == EWidgetCachePolicy::DestroyOnClose)
	{
		Widget->CloseWidgetAndChildren();
		ActiveWidgets.Remove(Tag);
	}
	else
	{
		Widget->SetVisibility(ESlateVisibility::Collapsed);
		Widget->ClearAllChildren();
		// Focus 획득 상태로 초기화
		Widget->OnFocusGainedAnim();
	}
}

void UUIManagerSubsystem::DefocusTopWidget()
{
	if (WidgetStack.IsEmpty())
	{
		return;
	}

	FGameplayTag TopTag = WidgetStack.Last();
	ApplyWidgetInputContext(ConfigData.WidgetMap[TopTag], false);

	// 기존 위젯의 Focus를 잃는 애니메이션을 실행합니다.
	if (UECUserWidget** OldTop = ActiveWidgets.Find(TopTag))
	{
		(*OldTop)->OnFocusLostAnim();
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
		// 이미 최상단 위젯이라면 굳이 재정렬과 IMC 스왑을 할 필요가 없습니다.
		if (WidgetStack.Last() == FoundTag)
		{
			return;
		}

		// 기존 위젯 Focus 제거
		DefocusTopWidget();

		// 선택된 위젯을 스택 최상단으로 이동
		WidgetStack.Remove(FoundTag);
		WidgetStack.Push(FoundTag);

		// 위젯의 Z-Order 갱신
		BringWidgetToFrontVisually(FocusedWidget);

		// 선택된 위젯에 Focus 부여
		FocusTopWidget();

		// 마우스 포커스 갱신 (선택된 위젯에 입력 포커스 강제)
		UpdateInputMode();
	}
}
