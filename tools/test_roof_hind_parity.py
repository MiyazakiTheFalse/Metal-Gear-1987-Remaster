"""Static cross-engine regression gates for the Building 1 roof continuation.

These assertions check committed source, NOT a UE compilation or a browser playtest.
"""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
BROWSER = (ROOT / "browser/game.js").read_text(encoding="utf-8")
MISSION = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGGrayFoxMission.cpp").read_text(encoding="utf-8")
HEADER = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Public/MGGrayFoxMission.h").read_text(encoding="utf-8")
SAVE = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Public/MGGrayFoxSaveGame.h").read_text(encoding="utf-8")
EXPORT = (ROOT / "browser/tools/export_field_rigs.cjs").read_text(encoding="utf-8")
RIG = (ROOT / "browser/assets/field_rigs.js").read_text(encoding="utf-8")


class RoofParitySourceTests(unittest.TestCase):
    def test_lift_is_earned_and_reversible(self):
        self.assertIn("stage===8||stage===9||stage===10)&&service.parachute", BROWSER)
        self.assertIn("bParachuteTaken && !bRoofActive && CanReach", MISSION)
        self.assertIn("bRoofActive && CanReach(Player, RoofReturnLift", MISSION)
        self.assertIn("roof.active=false;player.x=62.5", BROWSER)

    def test_both_engines_have_cover_and_timed_attack(self):
        self.assertIn("roof blast shelter", BROWSER)
        self.assertIn("B1RoofBlastShelter", MISSION)
        self.assertIn("roof.phase='telegraph';roof.clock=1.25", BROWSER)
        self.assertIn("HindPhase = 1; HindPhaseTime = 1.25f", MISSION)
        self.assertIn("lineBlockedSight({x:roof.flyX", BROWSER)
        self.assertIn("LineTraceSingleByChannel", MISSION)

    def test_ammo_and_hits_are_persistent(self):
        self.assertIn("roof.grenades--", BROWSER)
        self.assertIn("--RoofGrenadesRemaining", MISSION)
        self.assertIn("HindHitsRemaining = 3", HEADER)
        self.assertIn("RoofGrenadesRemaining = 6", HEADER)
        self.assertIn("Save->SchemaVersion = 6;", MISSION)
        self.assertIn("uint32 SchemaVersion = 6;", SAVE)
        self.assertIn("version:10,b2:", BROWSER)
        self.assertIn("bRoofCheckpoint", MISSION)
        self.assertIn("Save->bParachuteTaken && Save->bKidDefeated", MISSION)
        self.assertIn("if(data.version>=9&&stage>=9)", BROWSER)

    def test_cross_cast_facial_and_full_body_motion(self):
        self.assertIn("const facialBones=[...bones", RIG)
        self.assertIn("const skeleton=role=>facialBones", RIG)
        self.assertIn("rot[16]", RIG)
        self.assertIn("rot[17]", RIG)
        for clip in ["Aim:", "Cover:", "Melee:", "Reaching:", "Hit:", "RadioListen:", "CrouchAim:", "ProneAim:"]:
            self.assertIn(clip, EXPORT)
        self.assertIn("length:49", EXPORT)


if __name__ == "__main__":
    unittest.main()
