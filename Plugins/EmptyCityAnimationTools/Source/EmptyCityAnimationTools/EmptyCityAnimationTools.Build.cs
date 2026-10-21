using UnrealBuildTool;

public class EmptyCityAnimationTools : ModuleRules
{
	public EmptyCityAnimationTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"AnimationModifiers",
				"AnimationBlueprintLibrary"
			}
		);
	}
}
