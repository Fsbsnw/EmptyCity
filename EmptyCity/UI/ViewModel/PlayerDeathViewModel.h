#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/ViewModel/ECViewModelBase.h"
#include "PlayerDeathViewModel.generated.h"

class UTexture2D;

/** 플레이어를 죽인 적 타입에 맞는 사망 화면 데이터를 제공합니다. */
UCLASS()
class EMPTYCITY_API UPlayerDeathViewModel : public UECViewModelBase
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies(AActor* ContextActor) override;
	virtual void BroadcastInitialValues() override;

	UTexture2D* GetDeathScreenImage() const { return DeathScreenImage; }
	FGameplayTag GetKillerEnemyTag() const { return KillerEnemyTag; }
	UTexture2D* GetDroppedItemIcon() const { return DroppedItemIcon; }
	FText GetDroppedItemName() const { return DroppedItemName; }
	int32 GetDroppedItemCount() const { return DroppedItemCount; }

private:
	void SetDeathScreenImage(UTexture2D* NewImage);
	void SetKillerEnemyTag(FGameplayTag NewEnemyTag);
	void SetDroppedItemIcon(UTexture2D* NewIcon);
	void SetDroppedItemName(const FText& NewName);
	void SetDroppedItemCount(int32 NewCount);

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> DeathScreenImage;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	FGameplayTag KillerEnemyTag;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> DroppedItemIcon;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	FText DroppedItemName;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 DroppedItemCount = 0;
};
