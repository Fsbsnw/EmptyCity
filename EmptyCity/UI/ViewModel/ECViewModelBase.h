// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "ECViewModelBase.generated.h"

class APlayerController;
class APlayerState;
class APawn;

/**
 * ViewModel이 상호작 대상과 현재 로컬 플레이어 정보에 동시에 접근할 수 있게 하는 기반 클래스입니다.
 */
UCLASS()
class EMPTYCITY_API UECViewModelBase : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	/** 속성 값이 변경되었을 때 호출될 내부 콜백 함수들을 바인딩합니다.
	 * ContextActor는 상호작 대상이고, 플레이어 정보는 OwningWidget에서 별도로 해결합니다. */
	virtual void BindCallbacksToDependencies(AActor* ContextActor);
	/** 뷰가 처음 초기화될 때 현재 속성의 기본 값을 UI에 브로드캐스트합니다. */
	virtual void BroadcastInitialValues();

protected:
	/** 위젯을 열게 한 상호작 대상입니다. */
	AActor* GetContextActor() const { return CachedContextActor.Get(); }

	/** 상호작 대상과 무관하게 현재 위젯을 소유한 로컬 플레이어입니다. */
	APlayerController* GetPlayerController() const { return CachedPlayerController.Get(); }
	APawn* GetPlayerPawn() const { return CachedPlayerPawn.Get(); }
	APlayerState* GetPlayerState() const { return CachedPlayerState.Get(); }

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CachedContextActor;

	UPROPERTY(Transient)
	TWeakObjectPtr<APlayerController> CachedPlayerController;

	UPROPERTY(Transient)
	TWeakObjectPtr<APawn> CachedPlayerPawn;

	UPROPERTY(Transient)
	TWeakObjectPtr<APlayerState> CachedPlayerState;
};
