// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class AGameAboutWritting : ModuleRules
{
	public AGameAboutWritting(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Niagara", "GameplayTasks", "RHI", "ImageWrapper", "ImageCore", "MeshDescription",
            "StaticMeshDescription",
            "GeometryCore",
            "MeshUtilitiesCommon",
            "PhysicsCore",
            "RealtimeMeshComponent"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });


        // DLL copy logic
        string ProjectRoot = Path.GetFullPath(Path.Combine(Target.ProjectFile!.Directory!.FullName));

        // ----------------------------------//
        //              UDPIPE               //
        // ----------------------------------//
        string UDPIPE_DLL_NAME = "UDPIPE_AGMAABOUTWRITTING.dll";
        string UDPIPE_BINARY_DATA = "english-ewt-ud-2.5-191206.udpipe";
        string DllSourcePath = Path.Combine(ProjectRoot, "ExternalDll", UDPIPE_DLL_NAME); // Adjust path to your actual DLL location
        string LearntDataPathSource = Path.Combine(ProjectRoot, "ExternalDll", UDPIPE_BINARY_DATA);
        string DllDestPath = Path.Combine(ProjectRoot, "Binaries", "Win64", UDPIPE_DLL_NAME);
        string LearntDataDestPath = Path.Combine(ProjectRoot, "Binaries", "Win64", UDPIPE_BINARY_DATA);

        // Copy the DLL to the output folder
        if (!File.Exists(DllDestPath) || File.GetLastWriteTimeUtc(DllSourcePath) > File.GetLastWriteTimeUtc(DllDestPath))
        {
            File.Copy(DllSourcePath, DllDestPath, true);
            File.Copy(LearntDataPathSource, LearntDataDestPath, true);
        }

        RuntimeDependencies.Add("$(TargetOutputDir)/UDPIPE_AGMAABOUTWRITTING.dll", DllSourcePath);
        RuntimeDependencies.Add("$(TargetOutputDir)/english-ewt-ud-2.5-191206.udpipe", LearntDataPathSource);

        // ----------------------------------//
        //              ConceptNet           //
        // ----------------------------------//
        string CONCEPTNET_DLL_NAME = "ConceptRelationDll.dll";
        string CONCEPTNET_BINARY_DATA = "vector.bin";
        string CONCEPTNETDllSourcePath = Path.Combine(ProjectRoot, "ExternalDll", CONCEPTNET_DLL_NAME); // Adjust path to your actual DLL location
        string CONCEPTNETLearntDataPathSource = Path.Combine(ProjectRoot, "ExternalDll", CONCEPTNET_BINARY_DATA);
        string CONCEPTNETDllDestPath = Path.Combine(ProjectRoot, "Binaries", "Win64", CONCEPTNET_DLL_NAME);
        string CONCEPTNETLearntDataDestPath = Path.Combine(ProjectRoot, "Binaries", "Win64", CONCEPTNET_BINARY_DATA);

        // Copy the DLL to the output folder
        if (!File.Exists(CONCEPTNETDllDestPath) || File.GetLastWriteTimeUtc(CONCEPTNETDllSourcePath) > File.GetLastWriteTimeUtc(CONCEPTNETDllDestPath))
        {
            File.Copy(CONCEPTNETDllSourcePath, CONCEPTNETDllDestPath, true);
            File.Copy(CONCEPTNETLearntDataPathSource, CONCEPTNETLearntDataDestPath, true);
        }

        RuntimeDependencies.Add("$(TargetOutputDir)/ConceptRelationDll.dll", DllSourcePath);
        RuntimeDependencies.Add("$(TargetOutputDir)/vector.bin", LearntDataPathSource);


        //RuntimeDependencies.Add("$(BinaryOutputDir)/" + DllName);
        //RuntimeDependencies.Add("$(BinaryOutputDir)/" + LearntDataPath);

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
