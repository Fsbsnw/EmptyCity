#pragma once
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

#include "ECMessageTypes.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AddNotification_Message);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_Killed);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_RegionEntered);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_TimeChanged);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_ItemAcquired);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_ItemAcquired_Normal);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_ItemAcquired_Rare);

/**
 * 시간 변화, 아이템 획득 알림처럼 일시적으로 표시되는 알림 데이터를 전달합니다.
 */
USTRUCT(BlueprintType)
struct FECNotificationMessage
{
	GENERATED_BODY()

	/** 알림을 받을 채널을 저장합니다. */
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	FGameplayTag TargetChannel;

	/** 알림을 받을 플레이어를 저장합니다. (if none is set then it will display for all local players) */
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	TObjectPtr<APlayerState> TargetPlayer = nullptr;

	/** 알림에 추가적으로 표시할 메시지입니다. */
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	FText PayloadMessage;

	// Extra payload specific to the target channel (e.g., a style or definition asset)
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	FGameplayTag PayloadTag;

	// Extra payload specific to the target channel (e.g., a style or definition asset)
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	TObjectPtr<UObject> PayloadObject = nullptr;

	/** 알림에 추가적으로 전달할 값입니다. */ 
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	int32 PayloadValue = 0;

	/** 아이템 알림에 표시할 아이템 브러시입니다. */
	UPROPERTY(BlueprintReadOnly)
	FSlateBrush PayloadItemBrush;
};