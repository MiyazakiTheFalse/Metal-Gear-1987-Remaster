using System.IO;
using UnrealBuildTool;

public class MGRemaster : ModuleRules
{
    public MGRemaster(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore"
        });

        string RustRoot = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "..", "..", "rust"));
        PublicIncludePaths.Add(Path.Combine(RustRoot, "include"));
        bool withRust = false;
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string rustLib = Path.Combine(RustRoot, "target", "release", "mg_legacy_core.lib");
            if (File.Exists(rustLib))
            {
                PublicAdditionalLibraries.Add(rustLib);
                withRust = true;
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string rustLib = Path.Combine(RustRoot, "target", "release", "libmg_legacy_core.a");
            if (File.Exists(rustLib))
            {
                PublicAdditionalLibraries.Add(rustLib);
                withRust = true;
            }
        }

        // Switch native linking needs authorised Nintendo SDK and Unreal platform support.
        PublicDefinitions.Add("MG_WITH_RUST=" + (withRust ? "1" : "0"));
    }
}
