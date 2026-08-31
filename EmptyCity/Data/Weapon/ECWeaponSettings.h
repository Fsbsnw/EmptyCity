#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ECWeaponSettings.generated.h"

UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Weapon Settings"))
class EMPTYCITY_API UECWeaponSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category="공격 배율")
	float LightAttackDamageMultiplier;
	
	UPROPERTY(Config, EditAnywhere, Category="공격 배율")
	float HeavyAttackDamageMultiplier;
	
};
