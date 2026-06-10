#pragma once

#include "AbilitySystem/Ability/ECGameplayAbility.h"
#include "Interaction/InteractionOption.h"
#include "ECGameplayAbility_Interact.generated.h"

class UIndicatorDescriptor;
class UObject;
class UUserWidget;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;

/**
 * UECGameplayAbility_Interact
 *
 * 캐릭터의 "상호작용"을 담당하는 게임플레이 어빌리티입니다.
 *
 * 역할은 크게 세 가지입니다.
 * 
 *  1) 주변 상호작용 대상을 감시하는 태스크를 켭니다.
 *     → 플레이어가 상호작용 가능한 물체에 다가가면 해당 어빌리티를 미리 부여받습니다.
 *   
 *   
 *  2) 전방의 상호작용 대상을 감시하는 태스크를 켭니다.
 *     → 전방에 상호작용이 가능한 오브젝트가 존재하는 경우 FInteractionOption을 토대로 화면에 정보를 표시합니다.
 *     
 *  3) 플레이어가 상호작용 키를 누르면, 현재 선택지(CurrentOptions) 중 하나를
 *     게임플레이 이벤트로 실제 발동시킵니다.
 *
 */
UCLASS()
class UECGameplayAbility_Interact : public UECGameplayAbility
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// GameplayAbility Interface
// ─────────────────────────────────────────────────────────────	
public:
	UECGameplayAbility_Interact();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputStarted() override;
	
// ─────────────────────────────────────────────────────────────
// Task Activation
// ─────────────────────────────────────────────────────────────
protected:
	/** 주변의 상호작용 대상을 주기적으로 검사해, 필요한 어빌리티를 플레이어에게 미리 부여하는 태스크를 켭니다. */
	void StartNearbyAbilityGranting();
	
	/** 전방의 상호작용 대상을 주기적으로 검사해, 상호작용이 가능한 오브젝트가 있는지 탐색하는 태스크를 켭니다. */
	void StartLookForInteractables();


// ─────────────────────────────────────────────────────────────
// Internal Interaction Method
// ─────────────────────────────────────────────────────────────
private:	
	/** 현재 사용 가능한 상호작용 선택지 목록을 갱신합니다. 보통 감시 태스크의 변경 이벤트에서 호출됩니다. */
	UFUNCTION()
	void UpdateInteractions(const TArray<FInteractionOption>& InteractiveOptions);

	/**
	 * 플레이어가 상호작용을 실행할 때 호출됩니다. 현재 선택지 중 하나를 실제로 발동시킵니다.
	 * TimeWaited: WaitInputPress 태스크의 OnPress 델리게이트 시그니처에 맞추기 위한 파라미터입니다(미사용).
	 */
	UFUNCTION()
	void TriggerInteraction(float TimeWaited = 0.0f);

	
// ─────────────────────────────────────────────────────────────
// Interval, UI Variable
// ─────────────────────────────────────────────────────────────
protected:
	/** 주변 상호작용 대상을 검사하는 주기(초)입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|수치")
	float InteractionScanRate = 0.1f;

	/** 주변 상호작용 대상을 검사할 반경입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|수치")
	float InteractionScanRange = 500;

	/** 옵션이 자체 위젯을 지정하지 않았을 때 사용할 기본 상호작용 위젯 클래스입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|UI")
	TSoftClassPtr<UUserWidget> DefaultInteractionWidgetClass;
	
	
// ─────────────────────────────────────────────────────────────
// Cache Variable
// ─────────────────────────────────────────────────────────────
protected:
	/** 지금 상호작용할 수 있는 선택지 목록입니다. UpdateInteractions()로 갱신됩니다. */
	UPROPERTY(BlueprintReadWrite)
	TArray<FInteractionOption> CurrentOptions;

	/*/** 화면에 상호작용 위치를 표시해 주는 인디케이터(아이콘) 목록입니다. #1#
	UPROPERTY()
	TArray<TObjectPtr<UIndicatorDescriptor>> Indicators;*/

};
