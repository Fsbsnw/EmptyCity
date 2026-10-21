#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Subsystem/ECTutorialTypes.h"
#include "ECTutorialSettings.generated.h"

class UECTutorialWidget;
class UTexture2D;

/** 단계별 튜토리얼 이미지의 Canvas Panel 배치를 선택적으로 덮어씁니다. */
USTRUCT(BlueprintType)
struct FECTutorialImageLayout
{
	GENERATED_BODY()

	/** 활성화하면 WBP에 설정된 기본 위치 대신 Position을 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout")
	bool bOverridePosition = false;

	/** Image_Tutorial이 속한 Canvas Panel 기준 위치입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout",
		meta=(EditCondition="bOverridePosition"))
	FVector2D Position = FVector2D::ZeroVector;

	/** 활성화하면 WBP에 설정된 기본 크기 대신 Size를 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout")
	bool bOverrideSize = false;

	/** Image_Tutorial의 표시 크기입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout",
		meta=(EditCondition="bOverrideSize", ClampMin="1.0"))
	FVector2D Size = FVector2D(512.0, 256.0);
};

/** 튜토리얼 위젯과 단계별 이미지를 프로젝트 설정에서 지정합니다. */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Tutorial Settings"))
class EMPTYCITY_API UECTutorialSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** HUD 레이어에 생성할 튜토리얼 위젯 클래스입니다. */
	UPROPERTY(Config, EditAnywhere, Category="Tutorial")
	TSoftClassPtr<UECTutorialWidget> TutorialWidgetClass;

	/** 화면에 표시되는 단계별 이미지입니다. 대기/완료 단계는 비워둘 수 있습니다. */
	UPROPERTY(Config, EditAnywhere, Category="Tutorial")
	TMap<EECTutorialStage, TSoftObjectPtr<UTexture2D>> TutorialImages;

	/** 단계별 이미지 위치와 크기입니다. 등록하지 않은 단계는 WBP의 기본 배치를 사용합니다. */
	UPROPERTY(Config, EditAnywhere, Category="Tutorial")
	TMap<EECTutorialStage, FECTutorialImageLayout> TutorialImageLayouts;
};

