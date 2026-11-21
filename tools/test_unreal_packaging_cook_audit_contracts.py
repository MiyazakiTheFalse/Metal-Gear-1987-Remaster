"""Source-only contracts for UE5.7 packaging safety and crash diagnosis.

Unreal PIE/UBT/GLB import are NOT run by Python and must still be witnessed.
"""
import ast
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
read = lambda path: (ROOT / path).read_text(encoding="utf-8")


class UnrealPackagingContracts(unittest.TestCase):
    def test_cook_keeps_runtime_loaded_character_animation_weapon_roots(self):
        config = read("unreal/MGRemaster/Config/DefaultGame.ini")
        for folder in (
            "/Game/Characters",
            "/Game/Weapons",
            "/Game/Feedback",
            "/Game/Audio/Weapons",
        ):
            self.assertIn('+DirectoriesToAlwaysCook=(Path="' + folder + '")', config)
        self.assertIn("UnrealEd.ProjectPackagingSettings", config)

    def test_import_audit_source_parses_and_build_is_gate_locked(self):
        importer = read("unreal/MGRemaster/Content/Python/import_field_character_glbs.py")
        auditor = read("unreal/MGRemaster/Content/Python/audit_native_character_assets.py")
        ast.parse(importer)
        ast.parse(auditor)
        self.assertIn("MG_CHARACTER_IMPORT_PASS roles=", importer)
        self.assertIn("MG_NATIVE_CHARACTER_AUDIT_PASS roles=", auditor)
        self.assertIn("sequence.get_skeleton()", auditor)
        self.assertIn("get_bounds()", auditor)
        self.assertIn('get_editor_property("number_of_sampled_frames")', auditor)
        self.assertIn("required_clips(role)", auditor)
        build = read("scripts/build_windows.ps1")
        self.assertIn("verify_unreal_content.ps1", build)
        self.assertIn("unreal\\MGRemaster\\Content\\Characters", build)
        self.assertIn("BuildCookRun", build)
        self.assertLess(build.index("verify_unreal_content.ps1"), build.index("BuildCookRun"))

    def test_unreal_python_errors_cannot_be_confused_with_successful_import(self):
        for file, token in (
            ("scripts/import_unreal_content.ps1", "MG_CHARACTER_IMPORT_PASS roles=22"),
            ("scripts/verify_unreal_content.ps1", "MG_NATIVE_CHARACTER_AUDIT_PASS roles=22"),
        ):
            script = read(file)
            self.assertIn(token, script)
            self.assertIn("Select-String", script)
            self.assertIn("$EditorExitCode", script)
            self.assertIn("$OldPreference", script)
            self.assertIn("finally", script)
            self.assertIn("throw", script)

    def test_boss_skeletal_swap_clears_inherited_guard_anim_instance(self):
        src = read("unreal/MGRemaster/Source/MGRemaster/Private/MGGuardActor.cpp")
        start = src.index("bool AMGGuardActor::TryUseBespokeSkeletalArt")
        body = src[start:src.index("bool AMGGuardActor::CanAcceptDetention", start)]
        self.assertIn("SetAnimationMode(EAnimationMode::AnimationSingleNode)", body)
        self.assertLess(body.index("SetAnimationMode("),
                        body.index("SetSkeletalMeshAsset(Bespoke)"))
        self.assertIn("PrototypeAnimationRequest = NAME_None", body)

    def test_game_mode_does_not_spawn_duplicate_mission_writers(self):
        src = read("unreal/MGRemaster/Source/MGRemaster/Private/MGRemasterGameMode.cpp")
        self.assertIn("UGameplayStatics::GetActorOfClass(", src)
        self.assertIn("this, AMGGrayFoxMission::StaticClass()", src)
        self.assertIn("SpawnActor<AMGGrayFoxMission>", src)
        self.assertIn("duplicate", src.lower())

    def test_unrigged_guards_never_fire_invisible_weapon_proxies(self):
        src = read("unreal/MGRemaster/Source/MGRemaster/Private/MGGuardActor.cpp")
        self.assertIn("PistolProxy", src)
        self.assertIn("SecuritySidearm->SetStaticMesh(PistolProxy.Object)", src)
        self.assertIn("SecuritySidearm->SetRelativeScale3D(FVector::OneVector)", src)
        self.assertIn("SecuritySidearm->SetVisibility(SecuritySidearm->GetStaticMesh() != nullptr)", src)


if __name__ == "__main__":
    unittest.main()
