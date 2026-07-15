// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECNotificationDefinition.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "Subsystem/ECGameplayMessageSubsystem.h"
#include "ECNotificationHostWidget.generated.h"

class USizeBox;
struct FECNotificationMessage;

USTRUCT(BlueprintType)
struct FNotificationEntry
{
	GENERATED_BODY();

	/** 알림 위젯에서 쓰일 표시 정보를 가져올 데이터 행입니다. */
	UPROPERTY(BlueprintReadOnly)
	FNotificationDefinitionRow Row;

	/** 알림 위젯에 추가적으로 표시할 메시지입니다. */
	UPROPERTY(BlueprintReadOnly)
	FText Message;

	/** 알림 위젯에 추가적으로 표시할 값입니다. */
	UPROPERTY(BlueprintReadOnly)
	int32 Value = 0;

	/** 데이터 테이블에서 로드한 사운드 데이터를 관리합니다. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<USoundBase> Sound = nullptr;

	/** 데이터 테이블에서 로드한 아이콘 데이터를 관리합니다. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> Icon = nullptr;

	/** 화면에 표시되고 있는 알림 위젯을 관리합니다. */
	UPROPERTY()
	TObjectPtr<UUserWidget> AllocatedWidget = nullptr;
	
	/** 아이템 획득 알림에 표시되는 실제 아이템 브러시입니다. */
	UPROPERTY(BlueprintReadOnly)
	FSlateBrush ItemBrush;
};

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UECNotificationHostWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void DestroyNotificationWidget(UUserWidget* Widget);

	UFUNCTION(BlueprintImplementableEvent)
	UUserWidget* CreateNotificationWidget(const FNotificationEntry& Entry);
	
protected:
	/** 메시지 알림을 수신했을 때 실행하는 함수입니다. */
	void OnNotification(FGameplayTag Channel, const FECNotificationMessage& Notification);

	/** 알림 표시 정보를 담은 데이터 테이블입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Notification")
	TObjectPtr<UDataTable> NotificationDefinitionTable;

	/** 알림 타입을 설정하는 태그입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Notification")
	FGameplayTag NotificationType;

private:
	/** 등록된 메시지 리스너를 해제하기 위한 핸들입니다. */
	FECGameplayMessageListenerHandle ListenerHandle;

	/** 현재 생성되어 화면에 표시 중인 알림 목록입니다. */
	UPROPERTY(Transient)
	TArray<FNotificationEntry> ActiveEntries;
};