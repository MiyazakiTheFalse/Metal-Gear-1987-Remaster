"""Static narrative negative-space protection checks.

Run locally:
    python -m unittest discover -s tools -p 'test_*.py'

Source inspection is not Unreal 5.7 compilation, map playthrough, source-edition
proof, art approval or validation of an actually readable 720p in-game caption.
"""
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]
MISSION = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGGrayFoxMission.cpp").read_text(encoding="utf-8")
HEADER = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Public/MGGrayFoxMission.h").read_text(encoding="utf-8")
DOCTRINE = (ROOT / "docs/DOCTRINE_006_NARRATIVE_NEGATIVE_SPACE.md").read_text(encoding="utf-8")

class NegativeSpaceTests(unittest.TestCase):
    def test_no_floating_mandatory_lore_puzzle(self):
        self.assertIn("void AMGGrayFoxMission::AddWorldTrace(", MISSION)
        self.assertIn("bool AMGGrayFoxMission::TryInspectWorldTrace(", MISSION)
        self.assertIn("TWeakObjectPtr<AStaticMeshActor> Prop;", HEADER)
        self.assertIn("CanReach(Player, Trace.Prop.Get(), 185.f)", MISSION)
        section = MISSION.split("bool AMGGrayFoxMission::TryInspectWorldTrace(", 1)[1].split(
            "void AMGGrayFoxMission::CreatePrelude()", 1)[0]
        self.assertNotRegex(section, r"\bApplyAction\s*\(")
        self.assertNotRegex(section, r"\bSaveProgress\s*\(")
        self.assertNotRegex(section, r"\bSetFacilitySecurityState\s*\(")
        self.assertIn("DialogueTimeRemaining = 4.8f;", section)
        self.assertEqual(MISSION.count("return TryInspectWorldTrace(Player);"), 3)

    def test_five_actual_world_props_protect_story_secrets(self):
        names = (
            "CargoTransferManifest", "DutyRotationSlate", "CellRepairReceipt",
            "PropertyCustodyReceipt", "KennelDutyBoard"
        )
        self.assertEqual(MISSION.count("AddWorldTrace(TEXT("), 5)
        for name in names:
            self.assertIn('AddWorldTrace(TEXT("' + name + '")', MISSION)
        captions = re.findall(
            r'AddWorldTrace\(TEXT\("[^"]+"\),[\s\S]*?TEXT\("([^"]+)"\),\s*'
            r'(?:MG_FOX_[A-Z_]+|0u),\s*(?:MG_FOX_[A-Z_]+|0u)\);',
            MISSION
        )
        self.assertEqual(len(captions), 5)
        self.assertTrue(all(len(caption) <= 68 for caption in captions))
        for caption in captions:
            self.assertNotIn("BIG BOSS", caption)
            self.assertNotIn("VENOM", caption)
            self.assertNotIn("DIAMOND DOGS", caption)
            self.assertNotIn("METAL GEAR", caption)
        self.assertIn("MG_FOX_SLICE_COMPLETED, 0u", MISSION)
        self.assertIn("0u, MG_FOX_CAPTURED", MISSION)

    def test_original_script_is_not_mistaken_for_1987_msx(self):
        self.assertIn("**NES**", DOCTRINE)
        self.assertIn("NekuraHoka", DOCTRINE)
        self.assertIn("Japanese MSX2", DOCTRINE)
        self.assertIn("The Phantom Pain", DOCTRINE)
        self.assertIn("Schneider's genuine blind spots", DOCTRINE)
        self.assertIn("**Disjointedness that should *not* be preserved:**", DOCTRINE)
        self.assertIn("docs/DOCTRINE_000", DOCTRINE.replace("Doctrine 000", "docs/DOCTRINE_000"))

    def test_reveals_and_puzzle_authority_remain_in_main_mission(self):
        # Environmental inspection remains an optional last chance after the
        # real rescue/card/custody interactions, not before them.
        interact = MISSION.split("bool AMGGrayFoxMission::TryInteract(", 1)[1].split(
            "void AMGGrayFoxMission::BroadcastStructuralImpact", 1)[0]
        self.assertLess(
            interact.index("return ApplyAction(MG_FOX_RESCUE, Player);"),
            interact.index("if (!(Flags & MG_FOX_SHOTMAKER_ENCOUNTERED))"))
        self.assertLess(
            interact.index("return ApplyAction(MG_FOX_COMPLETE_SLICE, Player);"),
            interact.rindex("return TryInspectWorldTrace(Player);"))
        self.assertIn("MG_FOX_CAPTURE", interact)
        self.assertIn("MG_FOX_CARD3_RECOVERED", interact)

if __name__ == "__main__":
    unittest.main()
