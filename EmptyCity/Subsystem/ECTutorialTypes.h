#pragma once

#include "CoreMinimal.h"
#include "ECTutorialTypes.generated.h"

/** 플레이어가 진행 중인 튜토리얼 단계입니다. */
UENUM(BlueprintType)
enum class EECTutorialStage : uint8
{
	Movement UMETA(DisplayName = "Movement"),
	OpenMap UMETA(DisplayName = "Open Map"),
	AwaitFirstRegionTravel UMETA(DisplayName = "Await First Region Travel"),
	Sprint UMETA(DisplayName = "Sprint"),
	AttackAndGuard UMETA(DisplayName = "Attack And Guard"),
	// 기존 세이브의 Completed 값(7)을 유지합니다.
	Completed = 7 UMETA(DisplayName = "Completed")
};

