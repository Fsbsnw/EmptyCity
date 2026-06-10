// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/ECViewModelFactoryLibrary.h"

#include "UI/Widget/ECUserWidget.h"
#include "ECViewModelBase.h"

UECViewModelBase* UECViewModelFactoryLibrary::CreateViewModel(
	TSubclassOf<UECViewModelBase> ViewModelClass,
	AActor* ContextActor,
	UECUserWidget* OwningWidget)
{
	if (!ViewModelClass || !OwningWidget) 
	{
		return nullptr;
	}
	
	// 1. 뷰모델 생성 (OwningWidget을 Outer로 지정하여 메모리 생명주기 귀속)
	UECViewModelBase* NewVM = NewObject<UECViewModelBase>(OwningWidget, ViewModelClass);
    
	// 2. ContextActor를 바탕으로 뷰모델 내부 데이터 바인딩 초기화
	NewVM->BindCallbacksToDependencies(ContextActor);

	return NewVM;
}
