#include "ECGameplayTags.h"

namespace ECGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Ability, "Ability");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Action_Jump, "Ability.Type.Action.Jump");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Action_Sprint, "Ability.Type.Action.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Action_Crouch, "Ability.Type.Action.Crouch");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Action_Guard, "Ability.Type.Action.Guard");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Action_Attack, "Ability.Type.Action.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Action_Attack_Light, "Ability.Type.Action.Attack.Light");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Action_Attack_Heavy, "Ability.Type.Action.Attack.Heavy");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Action_Attack_Melee, "Ability.Type.Action.Attack.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Action_HitReaction_Knockback, "Ability.Type.Action.HitReaction.Knockback");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Skill_Damage_Slash, "Ability.Type.Skill.Damage.Slash");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Skill_Damage_SlashCombo, "Ability.Type.Skill.Damage.SlashCombo");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Skill_Damage_Kick, "Ability.Type.Skill.Damage.Kick");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Skill_Damage_LeapSlam, "Ability.Type.Skill.Damage.LeapSlam");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Skill_Damage_WhirlSlash, "Ability.Type.Skill.Damage.WhirlSlash");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Skill_Damage_LeapingWhirlSlash, "Ability.Type.Skill.Damage.LeapingWhirlSlash");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Skill_Buff_Elite, "Ability.Type.Skill.Buff.Elite");

	
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Ability_Type_Skill_Damage_Slash, "Cooldown.Ability.Type.Skill.Damage.Slash");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Ability_Type_Skill_Damage_SlashCombo, "Cooldown.Ability.Type.Skill.Damage.SlashCombo");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Ability_Type_Skill_Damage_Kick, "Cooldown.Ability.Type.Skill.Damage.Kick");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Ability_Type_Skill_Damage_LeapSlam, "Cooldown.Ability.Type.Skill.Damage.LeapSlam");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Ability_Type_Skill_Damage_WhirlSlash, "Cooldown.Ability.Type.Skill.Damage.WhirlSlash");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Ability_Type_Skill_Damage_LeapingWhirlSlash, "Cooldown.Ability.Type.Skill.Damage.LeapingWhirlSlash");
	

	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_Attack_Light, "InputTag.Ability.Attack.Light");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_Attack_Heavy, "InputTag.Ability.Attack.Heavy");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_Guard, "InputTag.Ability.Guard");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_Jump, "InputTag.Ability.Jump");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_Interact, "InputTag.Ability.Interact");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_Sprint, "InputTag.Ability.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_Crouch, "InputTag.Ability.Crouch");

	UE_DEFINE_GAMEPLAY_TAG(InputTag_QuickBar_Slot1, "InputTag.QuickBar.Slot1");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_QuickBar_Slot2, "InputTag.QuickBar.Slot2");

	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move, "InputTag.Move");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look_Mouse, "InputTag.Look.Mouse");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Crouch, "InputTag.Crouch");
	
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI, "InputTag.UI");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_HUD, "InputTag.UI.HUD");	
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Inventory, "InputTag.UI.Inventory");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Inventory_TradingPost, "InputTag.UI.Inventory.TradingPost");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Inventory_Storage, "InputTag.UI.Inventory.Storage");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Crafting, "InputTag.UI.Crafting");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Map, "InputTag.UI.Map");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Bed, "InputTag.UI.Bed");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Dream, "InputTag.UI.Dream");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Sleep, "InputTag.UI.Sleep");

	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_FadeScreen, "InputTag.UI.FadeScreen");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_FadeScreen_In, "InputTag.UI.FadeScreen.In");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_FadeScreen_Out, "InputTag.UI.FadeScreen.Out");
	
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Navigation, "InputTag.UI.Navigation");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Navigation_Confirm, "InputTag.UI.Navigation.Confirm");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Navigation_Escape, "InputTag.UI.Navigation.Escape");
	
	UE_DEFINE_GAMEPLAY_TAG(Movement_Mode_Walking, "Movement.Mode.Walking");
	UE_DEFINE_GAMEPLAY_TAG(Movement_Mode_Falling, "Movement.Mode.Falling");
	
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Death, "GameplayEvent.Death");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_DamageTaken, "GameplayEvent.DamageTaken");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Debuff_Stun, "GameplayEvent.Debuff.Stun");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Item_Use, "GameplayEvent.Item.Use");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Parried, "GameplayEvent.Parried");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_ParrySuccess, "GameplayEvent.ParrySuccess");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Attack_Melee_Hit, "GameplayEvent.Attack.Melee.Hit");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_HitReaction_Knockback, "GameplayEvent.HitReaction.Knockback");
	
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_DamageMultiplier, "SetByCaller.DamageMultiplier");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Heal, "SetByCaller.Heal");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Stamina, "SetByCaller.Stamina");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_StaminaDamage, "SetByCaller.StaminaDamage");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_WeaponAttackPower, "SetByCaller.WeaponAttackPower");
	
	UE_DEFINE_GAMEPLAY_TAG(Status_Moving, "Status.Moving");
	UE_DEFINE_GAMEPLAY_TAG(Status_Sprinting, "Status.Sprinting");
	UE_DEFINE_GAMEPLAY_TAG(Status_Attacking, "Status.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(Status_Crouching, "Status.Crouching");
	UE_DEFINE_GAMEPLAY_TAG(Status_Guarding, "Status.Guarding");
	UE_DEFINE_GAMEPLAY_TAG(Status_ParryWindow, "Status.ParryWindow");
	UE_DEFINE_GAMEPLAY_TAG(Status_Death, "Status.Death");
	UE_DEFINE_GAMEPLAY_TAG(Status_Death_Dying, "Status.Death.Dying");
	UE_DEFINE_GAMEPLAY_TAG(Status_Death_Dead, "Status.Death.Dead");

	UE_DEFINE_GAMEPLAY_TAG(Status_Buff, "Status.Buff");
	UE_DEFINE_GAMEPLAY_TAG(Status_Buff_EliteEmpowered, "Status.Buff.EliteEmpowered");

	UE_DEFINE_GAMEPLAY_TAG(Status_Debuff, "Status.Debuff");
	UE_DEFINE_GAMEPLAY_TAG(Status_Debuff_Stun, "Status.Debuff.Stun");

	UE_DEFINE_GAMEPLAY_TAG(MapNode_Location, "MapNode.Location");
	UE_DEFINE_GAMEPLAY_TAG(MapNode_Location_Shelter, "MapNode.Location.Shelter");
	UE_DEFINE_GAMEPLAY_TAG(MapNode_Location_Paradise, "MapNode.Location.Paradise");
	UE_DEFINE_GAMEPLAY_TAG(MapNode_Location_TradingPost, "MapNode.Location.TradingPost");
	UE_DEFINE_GAMEPLAY_TAG(MapNode_Location_Quarry, "MapNode.Location.Quarry");
	UE_DEFINE_GAMEPLAY_TAG(MapNode_Location_CultistBase, "MapNode.Location.CultistBase");
	UE_DEFINE_GAMEPLAY_TAG(MapNode_Location_NSeoulTower, "MapNode.Location.NSeoulTower");
	UE_DEFINE_GAMEPLAY_TAG(MapNode_Location_SeoulStation, "MapNode.Location.SeoulStation");
	
	const TMap<uint8, FGameplayTag> MovementModeTagMap =
	{
		{ MOVE_Walking, Movement_Mode_Walking },
		{ MOVE_Falling, Movement_Mode_Falling }
	};
	
	const TMap<uint8, FGameplayTag> CustomMovementModeTagMap =
	{
		
	};

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue, "GameplayCue");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat, "GameplayCue.Combat");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_HitReact, "GameplayCue.Combat.HitReact");

	UE_DEFINE_GAMEPLAY_TAG(Character_Type, "Character.Type");
	UE_DEFINE_GAMEPLAY_TAG(Character_Type_Player, "Character.Type.Player");
	UE_DEFINE_GAMEPLAY_TAG(Character_Type_Enemy, "Character.Type.Enemy");

	UE_DEFINE_GAMEPLAY_TAG(Weapon, "Weapon");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_WoodenClub, "Weapon.WoodenClub");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_PipeClub, "Weapon.PipeClub");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_SteelClub, "Weapon.SteelClub");

	UE_DEFINE_GAMEPLAY_TAG(Weapon_Zealot, "Weapon.Zealot");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Acolyte, "Weapon.Acolyte");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Boss, "Weapon.Boss");
	
	UE_DEFINE_GAMEPLAY_TAG(Tool, "Tool");
	UE_DEFINE_GAMEPLAY_TAG(Tool_Axe, "Tool.Axe");
	UE_DEFINE_GAMEPLAY_TAG(Tool_Pickaxe, "Tool.Pickaxe");
	
	UE_DEFINE_GAMEPLAY_TAG(Attack, "Attack");
	UE_DEFINE_GAMEPLAY_TAG(Attack_Normal, "Attack.Normal");
	UE_DEFINE_GAMEPLAY_TAG(Attack_Heavy, "Attack.Heavy");

	UE_DEFINE_GAMEPLAY_TAG(Cutscene, "Cutscene");
	UE_DEFINE_GAMEPLAY_TAG(Cutscene_Intro_1, "Cutscene.Intro.1");
	UE_DEFINE_GAMEPLAY_TAG(Cutscene_Intro_2, "Cutscene.Intro.2");
	UE_DEFINE_GAMEPLAY_TAG(Cutscene_Boss_Spawn, "Cutscene.Boss.Spawn");

	UE_DEFINE_GAMEPLAY_TAG(StateTree, "StateTree");
	UE_DEFINE_GAMEPLAY_TAG(StateTree_Event, "StateTree.Event");
	UE_DEFINE_GAMEPLAY_TAG(StateTree_Event_Patrol, "StateTree.Event.Patrol");
	UE_DEFINE_GAMEPLAY_TAG(StateTree_Event_Engage, "StateTree.Event.Engage");
	UE_DEFINE_GAMEPLAY_TAG(StateTree_Event_Combat, "StateTree.Event.Combat");
}
