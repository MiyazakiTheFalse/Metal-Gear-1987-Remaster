using UnrealBuildTool;
public class MGRemasterTarget : TargetRules
{
    public MGRemasterTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
        ExtraModuleNames.Add("MGRemaster");
    }
}
