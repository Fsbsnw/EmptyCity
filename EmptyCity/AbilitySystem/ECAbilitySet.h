#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ECAbilitySet.generated.h"

class UGameplayAbility;
class UECAbilitySystemComponent;

/** Ability - Tag를 매칭해놓은 게 핵심 */
USTRUCT()
struct FAbilitySet_GameplayAbility
{
	GENERATED_BODY()

public:
	/** 등록하고자 하는 Ability 입니다. */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> Ability = nullptr;
	
	/** 매칭하고자 하는 Tag입니다. */
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag;
	
	/** 특정 레벨에 사용 가능한 어빌리티임을 나타내기 위한 값입니다. */
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;
	
};

/**
 * AbilitySet을 통해 부여된 어빌리티 핸들을 보관하는 컨테이너.
 * TakeFromAbilitySystem을 호출하면 등록된 모든 핸들을 ASC에서 일괄 제거합니다.
 * 장비 해제 등 어빌리티를 묶음으로 회수할 때 사용합니다.
 */
USTRUCT()
struct FAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:	
	/** 핸들이 유효한 경우에만 AbilitySpecHandles에 추가합니다. */
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);

	/** 보관 중인 모든 핸들을 ASC에서 제거합니다. Authority가 없으면 무시됩니다. */
	void TakeFromAbilitySystem(UECAbilitySystemComponent* ASC);

protected:
	/** 실제 Handle을 관리하는 변수입니다. */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

};

UCLASS()
class EMPTYCITY_API UECAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// Ability Method
// ─────────────────────────────────────────────────────────────		
public:	
	/** ASC에 어빌리티를 부여합니다. OutGrantedHandles가 유효하면 핸들을 저장해 나중에 회수할 수 있습니다. */
	void GiveToAbilitySystem(UECAbilitySystemComponent* ASC, FAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr);


// ─────────────────────────────────────────────────────────────
// Ability Variable
// ─────────────────────────────────────────────────────────────	
private:	
	/** 등록하고자 하는 어빌리티 목록입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수")
	TArray<FAbilitySet_GameplayAbility> GrantedGameplayAbilities;
	
};