// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ECViewModelFactoryLibrary.generated.h"

class UECUserWidget;
class UECViewModelBase;
/**
 * 필요한 뷰 모델을 생성해주고 위젯에 주입해주는 블루프린트 라이브러리 클래스
 */
UCLASS()
class EMPTYCITY_API UECViewModelFactoryLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 블루프린트 및 내부 로직용 메인 함수 (cpp에 구현) */
	UFUNCTION(BlueprintCallable, Category = "ViewModel|Factory", meta = (DeterminesOutputType = "ViewModelClass", DefaultToSelf = "OwningWidget"))
	static UECViewModelBase* CreateViewModel(TSubclassOf<UECViewModelBase> ViewModelClass, AActor* ContextActor, UECUserWidget* OwningWidget);

	/** C++ 전용 편의성 템플릿 함수 */
	template<typename T>
	static T* CreateViewModel(AActor* ContextActor, UECUserWidget* OwningWidget)
	{
		// 안전장치: T가 반드시 UECViewModelBase의 자식이어야만 컴파일 가능
		static_assert(TIsDerivedFrom<T, UECViewModelBase>::IsDerived, "T must derive from UECViewModelBase");
        
		// 메인 함수 호출 후 자동으로 캐스팅해서 반환
		return Cast<T>(CreateViewModel(T::StaticClass(), ContextActor, OwningWidget));
	}
};
