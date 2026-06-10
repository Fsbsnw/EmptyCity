#include "ECGameplayTags.h"

namespace ECGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Action_Jump, "Ability.Type.Action.Jump");
  UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Action_Sprint, "Ability.Type.Action.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Action_Crouch, "Ability.Type.Action.Crouch");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_EnemyMeleeAttack, "Ability.Skill.EnemyMeleeAttack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_GrantEliteBuff, "Ability.Skill.GrantEliteBuff");

	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_Jump, "InputTag.Ability.Jump");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_Interact, "InputTag.Ability.Interact");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_Sprint, "InputTag.Ability.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_Crouch, "InputTag.Ability.Crouch");
	
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move, "InputTag.Move");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look_Mouse, "InputTag.Look.Mouse");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Crouch, "InputTag.Crouch");
	
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI, "InputTag.UI");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_HUD, "InputTag.UI.HUD");	
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Inventory, "InputTag.UI.Inventory");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Crafting, "InputTag.UI.Crafting");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Map, "InputTag.UI.Map");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Bed, "InputTag.UI.Bed");

	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_FadeScreen, "InputTag.UI.FadeScreen");	
	
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Navigation, "InputTag.UI.Navigation");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Navigation_Confirm, "InputTag.UI.Navigation.Confirm");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Navigation_Escape, "InputTag.UI.Navigation.Escape");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Navigation_Direction, "InputTag.UI.Navigation.Direction");
	
	UE_DEFINE_GAMEPLAY_TAG(Movement_Mode_Walking, "Movement.Mode.Walking");
	UE_DEFINE_GAMEPLAY_TAG(Movement_Mode_Falling, "Movement.Mode.Falling");
	
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Death, "GameplayEvent.Death");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_DamageTaken, "GameplayEvent.DamageTaken");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Debuff_Stun, "GameplayEvent.Debuff.Stun");
	
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Damage, "SetByCaller.Damage");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Heal, "SetByCaller.Heal");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Stamina, "SetByCaller.Stamina");

	UE_DEFINE_GAMEPLAY_TAG(Status_Moving, "Status.Moving");
	UE_DEFINE_GAMEPLAY_TAG(Status_Sprinting, "Status.Sprinting");
	UE_DEFINE_GAMEPLAY_TAG(Status_Attacking, "Status.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(Status_Crouching, "Status.Crouching");
	UE_DEFINE_GAMEPLAY_TAG(Status_Death, "Status.Death");
	UE_DEFINE_GAMEPLAY_TAG(Status_Death_Dying, "Status.Death.Dying");
	UE_DEFINE_GAMEPLAY_TAG(Status_Death_Dead, "Status.Death.Dead");

	UE_DEFINE_GAMEPLAY_TAG(Status_Buff, "Status.Buff");
	UE_DEFINE_GAMEPLAY_TAG(Status_Buff_EliteEmpowered, "Status.Buff.EliteEmpowered");

	UE_DEFINE_GAMEPLAY_TAG(Status_Debuff, "Status.Debuff");
	UE_DEFINE_GAMEPLAY_TAG(Status_Debuff_Stun, "Status.Debuff.Stun");

	UE_DEFINE_GAMEPLAY_TAG(MapNode_Location_Home, "MapNode.Location.Home");
	UE_DEFINE_GAMEPLAY_TAG(MapNode_Location_Paradise, "MapNode.Location.Paradise");
	UE_DEFINE_GAMEPLAY_TAG(MapNode_Location_TradingPost, "MapNode.Location.TradingPost");
	UE_DEFINE_GAMEPLAY_TAG(MapNode_Location_Quarry, "MapNode.Location.Quarry");
	UE_DEFINE_GAMEPLAY_TAG(MapNode_Location_Mine, "MapNode.Location.Mine");
	UE_DEFINE_GAMEPLAY_TAG(MapNode_Location_CultistBase, "MapNode.Location.CultistBase");
	
	const TMap<uint8, FGameplayTag> MovementModeTagMap =
	{
		{ MOVE_Walking, Movement_Mode_Walking },
		{ MOVE_Falling, Movement_Mode_Falling }
	};
	
	const TMap<uint8, FGameplayTag> CustomMovementModeTagMap =
	{
		
	};
}