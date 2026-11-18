using UnrealBuildTool;
public class MGRemasterEditorTarget : TargetRules
{
    public MGRemasterEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
        ExtraModuleNames.Add("MGRemaster");
    }
}
