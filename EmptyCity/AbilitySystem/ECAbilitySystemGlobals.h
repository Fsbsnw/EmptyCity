#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "ECAbilitySystemGlobals.generated.h"

UCLASS()
class EMPTYCITY_API UECAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Editor
// ─────────────────────────────────────────────────────────────
public:
#if WITH_EDITOR
	/** 에디터에서 전역 Attribute 기본값 캐시를 다시 생성합니다. */
	void ReloadAttributeDefaultsForEditor();
#endif
};
