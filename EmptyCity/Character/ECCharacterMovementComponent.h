#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ECCharacterMovementComponent.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_MovementStopped);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EMPTYCITY_API UECCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// CharacterMovementComponent Interface	
// ─────────────────────────────────────────────────────────────
public:
	virtual bool CanAttemptJump() const override;
	virtual float GetMaxSpeed() const override;
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	
};