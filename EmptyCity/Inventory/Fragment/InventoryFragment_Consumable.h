#pragma once

#include "CoreMinimal.h"
#include "Inventory/ECInventoryItemDefinition.h"
#include "InventoryFragment_Consumable.generated.h"

class UGameplayEffect;

UCLASS()
class EMPTYCITY_API UInventoryFragment_Consumable : public UECInventoryItemFragment
{
	GENERATED_BODY()
	
public:                                                                                                                                                                                                                           
	/** 사용 시 사용자에게 적용할 GameplayEffect. (HP회복 GE / Stamina회복 GE 등 아이템마다 다름) */                                                                                                                              
	UPROPERTY(EditDefaultsOnly, Category = "변수")                                                                                                                                                                                
	TSubclassOf<UGameplayEffect> EffectToApply;                                                                                                                                                                                   
                                                                                                                                                                                                                                    
	/** 1회 사용 시 소모할 개수입니다. */                                                                                                                                                                                         
	UPROPERTY(EditDefaultsOnly, Category = "변수")                                                                                                                                                                                
	int32 ConsumeCount = 1;         
	
};  