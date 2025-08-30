// Copyright Epic Games, Inc. All Rights Reserved.
using System.IO; // ADD THIS IMPORT FOR ONNX SUPPORT, 4.27 No plug-in found
using UnrealBuildTool;

public class NeuralNexusThesis : ModuleRules
{
	public NeuralNexusThesis(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "NavigationSystem", "UMG", "RenderCore", "RHI" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        // ONNX Runtime integration from nuget
        string BaseDir = Path.GetDirectoryName(ModuleDirectory);
        string OnnxPath = Path.Combine(BaseDir, "ThirdParty", "ONNXRuntime");
        string IncludePath = Path.Combine(OnnxPath, "include");
        string LibraryPath = Path.Combine(OnnxPath, "lib");

        PublicIncludePaths.Add(IncludePath);

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "onnxruntime.lib"));

            // Copy DLL to output directory
            string DllPath = Path.Combine(OnnxPath, "bin", "onnxruntime.dll");
            RuntimeDependencies.Add("$(TargetOutputDir)/onnxruntime.dll", DllPath);
        }

        bEnableExceptions = true;

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
