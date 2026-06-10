#include "ECMoveSpeedSet.h"

UECMoveSpeedSet::UECMoveSpeedSet()
	: MoveSpeed(300.0f)
	, SprintSpeed(360.0f)
{
}

void UECMoveSpeedSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetMoveSpeedAttribute() || Attribute == GetSprintSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}
