#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ECPlayerState.generated.h"

class UGameplayEffect;
class UECAbilitySystemComponent;
class UECAbilitySet;

/** 컨트롤러의 HUD 초기화 타이밍(PlayerStat 초기값 설정)을 전달하는 델리게이트입니다. */
DECLARE_MULTICAST_DELEGATE(FOnStatsInitializedDelegate);

UENUM(BlueprintType)
enum class EPlayerStatType : uint8
{
	Health		UMETA(DisplayName = "체력"),
	Stamina		UMETA(DisplayName = "스태미나"),
	MoveSpeed	UMETA(DisplayName = "이동속도"),
	MAX			UMETA(Hidden)
};

UCLASS()
class EMPTYCITY_API AECPlayerState : public APlayerState
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// PlayerState Interface
// ─────────────────────────────────────────────────────────────	
public:
	AECPlayerState();
	virtual void BeginPlay() override;
	

// ─────────────────────────────────────────────────────────────
// Stat Method
// ─────────────────────────────────────────────────────────────
public:
	/** 플레이어의 모든 스탯을 레벨에 따라 초기화하는 함수입니다. */
	void InitStats();

	/** 해당 스탯 레벨을 InLevel만큼 올리고 어트리뷰트를 갱신합니다. */
	void LevelUpStat(EPlayerStatType StatType, int32 InLevel = 1);

	/** 플레이어의 스탯 초기화 여부를 확인하는 Getter 함수입니다. */
	FORCEINLINE bool IsStatsInitialized() const { return bIsStatsInitialized; }
	
	/** 플레이어의 스탯 초기화 작업이 완료된 후에 호출되는 델리게이트입니다. */
	FOnStatsInitializedDelegate OnStatsInitializedDelegate;

private:
	/** 어트리뷰트 그룹 하나를 지정된 레벨로 초기화합니다. */
	void ApplyStatInit(EPlayerStatType StatType, int32 Level, bool bInitialInit);

	/** 해당 스탯의 현재 값을 Max 값에 맞춰 동기화합니다. (초기화/레벨업 시 풀회복 용도) */
	void SyncCurrentToMax(EPlayerStatType StatType);

	/** 플레이어의 스탯 초기화 여부를 판단하는 변수입니다. */
	bool bIsStatsInitialized = false;


// ─────────────────────────────────────────────────────────────
// Ability Method
// ─────────────────────────────────────────────────────────────
private:
	/** 플레이어가 기본으로 사용하는 어빌리티를 등록하는 함수입니다. */
	void InitAbilities();
	
	
// ─────────────────────────────────────────────────────────────
// Component
// ─────────────────────────────────────────────────────────────		
protected:
	/** 플레이어의 AbilitySystemComponent입니다. OnPossess 호출 시점에 동기화됩니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UECAbilitySystemComponent> ASC;
	
	
// ─────────────────────────────────────────────────────────────
// Ability Variable
// ─────────────────────────────────────────────────────────────
private:
	/** 플레이어가 기본으로 사용하는 어빌리티에 대한 정보입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|어빌리티")
	TArray<TObjectPtr<UECAbilitySet>> DefaultAbilitySets;


// ─────────────────────────────────────────────────────────────
// Stat Variable
// ─────────────────────────────────────────────────────────────
private:
	UPROPERTY()
	TMap<EPlayerStatType, int32> StatLevels;


// ─────────────────────────────────────────────────────────────
// Getter
// ─────────────────────────────────────────────────────────────
public:
	FORCEINLINE UECAbilitySystemComponent* GetAbilitySystemComponent() const { return ASC; }
	
};
