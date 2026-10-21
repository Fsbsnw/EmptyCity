#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "ECEnemyTableRow.generated.h"

class AECEnemyCharacterBase;
class UECHitVFXAttachmentProfile;
class UTexture2D;

/** Enemy C++ Class와 해당 적을 식별하는 GameplayTag의 매핑입니다. */
USTRUCT(BlueprintType)
struct FECEnemyTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 이 행이 나타내는 Enemy Character C++ Class입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy")
	TSubclassOf<AECEnemyCharacterBase> EnemyClass;

	/** 알림 등의 데이터 조회에 사용할 적 식별 태그입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy")
	FGameplayTag EnemyTag;

	/** 체력바 등 UI에 표시할 적 이름입니다. 비어 있으면 EnemyTag의 마지막 이름을 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy")
	FText EnemyName;

	/** 이 적에게 플레이어가 사망했을 때 사망 화면에 표시할 이미지입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy|Death Screen")
	TSoftObjectPtr<UTexture2D> PlayerDeathScreenImage;

	/** 이 적의 스켈레톤에서 사용할 피격 이펙트 본 재지정 및 오프셋 프로필입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy|Combat Feedback")
	TSoftObjectPtr<UECHitVFXAttachmentProfile> HitVFXAttachmentProfile;
};
