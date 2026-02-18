using UnrealBuildTool;

public class AGameAboutWrittingEditor : ModuleRules
{
    public AGameAboutWrittingEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Public = ONLY modules safe for runtime
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine"
        });

        // Private = ALL editor-only modules
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "UnrealEd",
            "EditorFramework",
            "Kismet",
            "KismetCompiler",
            "BlueprintGraph",
            "AssetRegistry",
            "Projects",
            "ApplicationCore",
            "InputCore",
            "Slate",
            "SlateCore"
        });

        PrivateDefinitions.Add("WITH_EDITOR=1");
    }
}