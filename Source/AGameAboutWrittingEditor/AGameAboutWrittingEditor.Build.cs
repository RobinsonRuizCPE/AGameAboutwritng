using UnrealBuildTool;

public class AGameAboutWrittingEditor : ModuleRules
{
    public AGameAboutWrittingEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // This is an EDITOR module → only load inside editor
        if (Target.bBuildEditor == false)
        {
            throw new System.Exception("AGameAboutWrittingEditor can only be built in editor configurations.");
        }

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "UnrealEd",
            "Kismet",
            "KismetCompiler",
            "BlueprintGraph",
            "AssetRegistry",
            "Slate",
            "SlateCore",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "EditorFramework",
            "UnrealEd",
            "Projects",
            "ApplicationCore",
            "InputCore"
        });
    }
}
