#include "ECGameplayAbility_FromEquipment.h"
#include "ECEquipmentInstance.h"

UECEquipmentInstance* UECGameplayAbility_FromEquipment::GetAssociatedEquipment() const
{
	// 어빌리티를 부여할 때 EquipmentInstance를 SourceObject로 넣어 두었으므로, 현재 Spec에서 그것을 다시 꺼낸다.
	if (FGameplayAbilitySpec* Spec = UGameplayAbility::GetCurrentAbilitySpec())
	{
		return Cast<UECEquipmentInstance>(Spec->SourceObject.Get());
	}
	
	return nullptr;
}

UECEquipmentInstance* UECGameplayAbility_FromEquipment::GetAssociatedItem() const
{
	// Instigator는 QuickBarComponent에서 부여합니다.
	if (UECEquipmentInstance* Equipment = GetAssociatedEquipment())
	{
		return Cast<UECEquipmentInstance>(Equipment->Instigator);
	}
	
	return nullptr;
}
