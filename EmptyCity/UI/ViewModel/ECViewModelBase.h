// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "ECViewModelBase.generated.h"

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECViewModelBase : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	/** 속성 값이 변경되었을 때 호출될 내부 콜백 함수들을 바인딩합니다.
	 * override된 클래스에서 ContextActor의 데이터를 바인딩 해야 합니다. */
	virtual void BindCallbacksToDependencies(AActor* ContextActor);
	/** 뷰가 처음 초기화될 때 현재 속성의 기본 값을 UI에 브로드캐스트합니다. */
	virtual void BroadcastInitialValues();
};
