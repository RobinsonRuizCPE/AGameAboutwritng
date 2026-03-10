// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class AGameAboutWritting : ModuleRules
{
	public AGameAboutWritting(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine","Renderer", "RenderCore", "InputCore", "EnhancedInput", "UMG", "Niagara", "GameplayTasks", "RHI", "RHICore", "ImageWrapper", "ImageCore", "MeshDescription",
            "StaticMeshDescription",
            "GeometryCore",
            "MeshUtilitiesCommon",
            "PhysicsCore",
            "ProceduralMeshComponent",
			"HTTP",
			"Json",
			"JsonUtilities"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore"});


        string ProjectRoot = Path.GetFullPath(Path.Combine(Target.ProjectFile!.Directory!.FullName));

        // ----------------------------------//
        //              UDPIPE               //
        // ----------------------------------//
        string UDPIPE_DLL_NAME = "UDPIPE_AGMAABOUTWRITTING.dll";
        string UDPIPE_BINARY_DATA = "english-ewt-ud-2.5-191206.udpipe";

        string DllSourcePath = Path.Combine(ProjectRoot, "ExternalDll", UDPIPE_DLL_NAME);
        string LearntDataPathSource = Path.Combine(ProjectRoot, "ExternalDll", UDPIPE_BINARY_DATA);

        RuntimeDependencies.Add("$(TargetOutputDir)/" + UDPIPE_DLL_NAME, DllSourcePath, StagedFileType.NonUFS);
        RuntimeDependencies.Add("$(TargetOutputDir)/" + UDPIPE_BINARY_DATA, LearntDataPathSource, StagedFileType.NonUFS);

        // ----------------------------------//
        //            ConceptNet             //
        // ----------------------------------//
        string CONCEPTNET_DLL_NAME = "ConceptRelationDll.dll";
        string CONCEPTNET_BINARY_DATA = "vector.bin";

        string CONCEPTNETDllSourcePath = Path.Combine(ProjectRoot, "ExternalDll", CONCEPTNET_DLL_NAME);
        string CONCEPTNETLearntDataPathSource = Path.Combine(ProjectRoot, "ExternalDll", CONCEPTNET_BINARY_DATA);

        RuntimeDependencies.Add("$(TargetOutputDir)/" + CONCEPTNET_DLL_NAME, CONCEPTNETDllSourcePath, StagedFileType.NonUFS);
        RuntimeDependencies.Add("$(TargetOutputDir)/" + CONCEPTNET_BINARY_DATA, CONCEPTNETLearntDataPathSource, StagedFileType.NonUFS);


        //RuntimeDependencies.Add("$(BinaryOutputDir)/" + DllName);
        //RuntimeDependencies.Add("$(BinaryOutputDir)/" + LearntDataPath);

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
