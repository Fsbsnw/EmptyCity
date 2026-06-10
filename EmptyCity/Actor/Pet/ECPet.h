#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/IInteractableTarget.h"
#include "ECPet.generated.h"

UCLASS()
class EMPTYCITY_API AECPet : public AActor, public IInteractableTarget
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// InteractableTarget Interface
// ─────────────────────────────────────────────────────────────		
public:
	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& OptionBuilder) override;

	
// ─────────────────────────────────────────────────────────────
// Interaction
// ─────────────────────────────────────────────────────────────	
protected:
	UPROPERTY(EditAnywhere, Category = "변수|상호작용")
	FInteractionOption Option;
	
};
