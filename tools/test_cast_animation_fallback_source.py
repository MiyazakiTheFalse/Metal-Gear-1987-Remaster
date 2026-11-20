"""Structural cross-engine animation/source presentation regression.

These assertions do not prove compiled UE5.7, real WebGL GLB rendering,
AnimationBlueprint production quality or playtest performance.
"""
import ast
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "unreal/MGRemaster/Source/MGRemaster"
content = lambda path: (ROOT / path).read_text(encoding="utf-8")
COMMON = content("unreal/MGRemaster/Source/MGRemaster/Public/MGPrototypeAnimationFallback.h")
PLAYER = content("unreal/MGRemaster/Source/MGRemaster/Private/MGPlayerCharacter.cpp")
GUARD = content("unreal/MGRemaster/Source/MGRemaster/Private/MGGuardActor.cpp")
FOX = content("unreal/MGRemaster/Source/MGRemaster/Private/MGPrisonerCharacter.cpp")
SHOTMAKER = content("unreal/MGRemaster/Source/MGRemaster/Private/MGShootGunnerActor.cpp")
KID = content("unreal/MGRemaster/Source/MGRemaster/Private/MGMachinegunKid.cpp")
BROWSER = content("browser/game.js")
GLB = content("browser/rigged_characters.js")
INTENT = content("browser/animation_intent.js")
INDEX = content("browser/index.html")
IMPORTER = content("unreal/MGRemaster/Content/Python/import_field_character_glbs.py")


class CastAnimationSourceTests(unittest.TestCase):
    def test_role_accurate_local_glb_not_streamed_generic_soldier(self):
        self.assertNotIn("MODEL_URL", GLB)
        self.assertNotIn("cktang88", GLB)
        for name in ("snake", "fox", "guard", "shoot_gunner",
                     "machinegun_kid", "pow_informant"):
            self.assertIn("'assets/models/" + name + ".glb'", GLB)
            self.assertIn('"' + name + '": (', IMPORTER)
        self.assertIn("cloneSkeleton(source.scene)", GLB)
        self.assertIn("window.MGAnimationIntent", GLB)
        self.assertIn("actor.kind", GLB)
        self.assertIn("intent.select(actor,speed,time)", GLB)
        self.assertIn("camera.lookAt(...view.target)", GLB)
        self.assertIn("!o.solid&&!o.cameraSolid", GLB)
        self.assertIn('src="animation_intent.js"', INDEX)
        self.assertIn("activeCastForRig(t)", BROWSER)
        self.assertIn("kind:'shoot_gunner'", BROWSER)
        self.assertIn("kind:'pow_informant'", BROWSER)
        self.assertIn("kind:machinegun?'machinegun_kid':'guard'", BROWSER)
        self.assertIn("dt,time:t,camera:", BROWSER)

    def test_animation_selection_has_priority_not_art_written_ai(self):
        for name in ("Down","Hit","Melee","Reaching","ProneAim",
                     "CrouchAim","IdleAlert","IdleLook","IdleWary",
                     "Address","Shoulder","Recoil","Search"):
            self.assertIn("'" + name + "'", INTENT)
        for forbidden in ("ApplyDamage", "SaveProgress", "ApplyAction",
                          "localStorage.setItem"):
            self.assertNotIn(forbidden, INTENT)

    def test_unreal_source_animation_fallback_never_overrides_authored_anim_bp(self):
        for term in ("GetAnimClass() != nullptr",
                     "EAnimationMode::AnimationBlueprint",
                     "PlayAnimation(Clip, IsLoop(Requested))",
                     "SetPlayRate", "FindClip", "LastRequest"):
            self.assertIn(term, COMMON)
        for forbidden in ("ApplyDamage", "SaveProgress", "SetMissionFlags"):
            self.assertNotIn(forbidden, COMMON)
        for name in ("Walk","Run","Prone","Crouch","Hit","Aim",
                     "RadioListen","Melee","IdleLook","IdleWary"):
            self.assertIn('TEXT("' + name + '")', PLAYER)
        self.assertIn("UpdatePrototypeFallbackAnimation();", PLAYER)
        self.assertIn("GetVelocity().Size2D()", PLAYER)
        self.assertIn("GetTimeSeconds()", PLAYER)
        self.assertIn("bBinocularView ? 34.f", PLAYER)
        self.assertIn("UpdatePrototypeFallbackAnimation();", GUARD)
        self.assertIn("ActualGroundSpeedCmPerSec", GUARD)
        self.assertIn("ApplyIncapacitatedVisual();", GUARD)
        self.assertIn("GetAnimClass() == nullptr", GUARD)
        self.assertIn('TEXT("Captive")', FOX)
        self.assertIn('TEXT("IdleWary")', FOX)
        self.assertIn("UpdatePrototypeFallbackAnimation();", SHOTMAKER)
        self.assertIn('TEXT("Shoulder")', SHOTMAKER)
        self.assertIn('TEXT("Recoil")', SHOTMAKER)
        self.assertIn("PlayPrototypeActing(Acting)", KID)
        self.assertIn("SetPrototypeAnimationSource(", KID)

    def test_unreal_importer_remains_valid_python_and_not_fake_asset_manifest(self):
        ast.parse(IMPORTER)
        self.assertIn("unreal.SkeletalMesh", IMPORTER)
        self.assertIn("unreal.AnimSequence", IMPORTER)
        self.assertIn("expected_names.update", IMPORTER)
        self.assertIn("not production art", IMPORTER.lower())


if __name__ == "__main__":
    unittest.main()
