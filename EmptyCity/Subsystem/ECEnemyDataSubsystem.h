#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ECEnemyDataSubsystem.generated.h"

class AECEnemyCharacterBase;
class UDataTable;
class UECHitVFXAttachmentProfile;
class UTexture2D;

/** Enemy Character Class를 기준으로 적 식별 데이터를 제공합니다. */
UCLASS()
class EMPTYCITY_API UECEnemyDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** WorldContextObject으로 ECEnemyDataSubsystem을 반환합니다. */
	static UECEnemyDataSubsystem& Get(const UObject* WorldContextObject);

	/** 실제 클래스의 최초 Native C++ 클래스에 대응하는 GameplayTag를 반환합니다. */
	FGameplayTag FindEnemyTag(UClass* EnemyClass) const;

	/** 블루프린트에서 Enemy Character Class에 대응하는 GameplayTag를 조회합니다. */
	UFUNCTION(BlueprintPure, Category="Enemy")
	FGameplayTag GetEnemyTag(TSubclassOf<AECEnemyCharacterBase> EnemyClass) const;

	/** 실제 클래스의 최초 Native C++ 클래스에 대응하는 표시 이름을 반환합니다. */
	FText FindEnemyDisplayName(UClass* EnemyClass) const;

	/** 블루프린트에서 Enemy Character Class에 대응하는 표시 이름을 조회합니다. */
	UFUNCTION(BlueprintPure, Category="Enemy")
	FText GetEnemyDisplayName(TSubclassOf<AECEnemyCharacterBase> EnemyClass) const;

	/** Enemy Character Class에 대응하는 플레이어 사망 화면 이미지를 반환합니다. */
	UTexture2D* FindPlayerDeathScreenImage(UClass* EnemyClass) const;

	UFUNCTION(BlueprintPure, Category="Enemy")
	UTexture2D* GetPlayerDeathScreenImage(TSubclassOf<AECEnemyCharacterBase> EnemyClass) const;

	/** Enemy Character Class에 대응하는 피격 이펙트 부착 프로필을 반환합니다. */
	UECHitVFXAttachmentProfile* FindHitVFXAttachmentProfile(UClass* EnemyClass) const;

	UFUNCTION(BlueprintPure, Category="Enemy")
	UECHitVFXAttachmentProfile* GetHitVFXAttachmentProfile(
		TSubclassOf<AECEnemyCharacterBase> EnemyClass) const;

private:
	/** Blueprint 클래스를 데이터 테이블에 등록된 Native Enemy 클래스로 변환합니다. */
	static UClass* ResolveNativeEnemyClass(UClass* EnemyClass);

	/** 프로젝트 설정을 통해 로드한 Enemy Data Table입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> EnemyDataTable;

	/** Native Enemy C++ Class 포인터를 EnemyTag로 변환하는 캐시입니다. */
	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, FGameplayTag> EnemyClassToTag;

	/** Native Enemy C++ Class 포인터를 UI 표시 이름으로 변환하는 캐시입니다. */
	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, FText> EnemyClassToDisplayName;

	/** Native Enemy C++ Class 포인터를 사망 화면 이미지로 변환하는 캐시입니다. */
	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, TSoftObjectPtr<UTexture2D>> EnemyClassToDeathScreenImage;

	/** Native Enemy C++ Class 포인터를 피격 이펙트 부착 프로필로 변환하는 캐시입니다. */
	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, TSoftObjectPtr<UECHitVFXAttachmentProfile>> EnemyClassToHitVFXAttachmentProfile;
};
