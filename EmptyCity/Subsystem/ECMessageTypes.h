#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "Styling/SlateBrush.h"

#include "ECMessageTypes.generated.h"

class APlayerState;
class APlayerController;
class UECInventoryItemDefinition;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AddNotification_Message);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Player_Death);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Boss_HalfHealthReached);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Boss_Death);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Tutorial_Input);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_Killed);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_RegionEntered);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_TimeChanged);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_ItemAcquired);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_ItemAcquired_Normal);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_ItemAcquired_Rare);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_ContentUnlocked);

/** 플레이어 행동이 게임 규칙에 의해 제한됐을 때 표시하는 경고 알림입니다. */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_Warning);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_Warning_RegionTravelDenied);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Notification_Warning_InventoryWeightLimit);

/**
 * 플레이어 사망이 시작되었음을 게임 흐름에 알립니다.
 */
USTRUCT(BlueprintType)
struct FECPlayerDeathMessage
{
	GENERATED_BODY()

	/** 사망한 플레이어 캐릭터입니다. */
	UPROPERTY(BlueprintReadOnly, Category="Player Death")
	TObjectPtr<AActor> PlayerActor = nullptr;

	/** 사망 화면을 표시할 로컬 플레이어 컨트롤러입니다. */
	UPROPERTY(BlueprintReadOnly, Category="Player Death")
	TObjectPtr<APlayerController> PlayerController = nullptr;

	/** 마지막 피해를 가한 액터입니다. 사망 화면 이미지 결정에 사용합니다. */
	UPROPERTY(BlueprintReadOnly, Category="Player Death")
	TObjectPtr<AActor> DamageInstigator = nullptr;

	/** 사망 시 인벤토리에서 잃은 아이템입니다. */
	UPROPERTY(BlueprintReadOnly, Category="Player Death")
	TSubclassOf<UECInventoryItemDefinition> DroppedItemDefinition;

	/** 사망 시 잃은 아이템의 수량입니다. */
	UPROPERTY(BlueprintReadOnly, Category="Player Death")
	int32 DroppedItemCount = 0;
};

/**
 * 보스 체력이 절반 이하에 도달했음을 게임 흐름에 알립니다.
 */
USTRUCT(BlueprintType)
struct FECBossHalfHealthReachedMessage
{
	GENERATED_BODY()

	/** 임계점에 도달한 보스입니다. */
	UPROPERTY(BlueprintReadWrite, Category="Boss Phase")
	TObjectPtr<AActor> BossActor = nullptr;

	/** 보스 페이즈 전환에 사용할 컷씬 태그입니다. */
	UPROPERTY(BlueprintReadWrite, Category=Cutscene)
	FGameplayTag CutsceneTag;

	/** 이벤트 발생 시점의 체력 비율입니다. */
	UPROPERTY(BlueprintReadWrite, Category="Boss Phase")
	float HealthRatio = 0.0f;
};

/**
 * 보스의 사망이 시작되었음을 게임 흐름에 알립니다.
 */
USTRUCT(BlueprintType)
struct FECBossDeathMessage
{
	GENERATED_BODY()

	/** 사망한 보스입니다. */
	UPROPERTY(BlueprintReadWrite, Category="Boss Death")
	TObjectPtr<AActor> BossActor = nullptr;

	/** 보스 사망 시 사용할 컷씬 태그입니다. */
	UPROPERTY(BlueprintReadWrite, Category=Cutscene)
	FGameplayTag CutsceneTag;
};

/** 플레이어가 튜토리얼 대상 입력을 실행했음을 전달합니다. */
USTRUCT(BlueprintType)
struct FECTutorialInputMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Tutorial")
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Tutorial")
	FGameplayTag InputTag;
};

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
