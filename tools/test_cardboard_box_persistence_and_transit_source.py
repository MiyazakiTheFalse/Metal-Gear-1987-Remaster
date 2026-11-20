"""Source-only checks for reusable original Metal Gear box and intentional freight breather.

Run locally: python -m unittest tools.test_cardboard_box_persistence_and_transit_source
This is NOT a runtime test, Unreal compilation or first-person playtest.
"""
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
UNREAL = ROOT / "unreal/MGRemaster/Source/MGRemaster"
EQUIP = (UNREAL / "Private/MGEquipmentComponent.cpp").read_text()
PLAYER = (UNREAL / "Private/MGPlayerCharacter.cpp").read_text()
MISSION = (UNREAL / "Private/MGGrayFoxMission.cpp").read_text()
GUARD = (UNREAL / "Private/MGGuardActor.cpp").read_text()
SAVE = (UNREAL / "Public/MGGrayFoxSaveGame.h").read_text()
WEB = (ROOT / "browser/game.js").read_text()
LINES = (ROOT / "browser/dialogue.js").read_text()
CONTRACT = json.loads((ROOT / "data/MSX2_FULL_CAMPAIGN_CONVERSION_PLAN_1995.json").read_text())
INFLUENCES = json.loads((ROOT / "data/SIDE_GAME_INFLUENCE_ADAPTATION_MATRIX.json").read_text())

class CardboardBoxAndFreightPacingSourceTests(unittest.TestCase):
    def test_original_box_is_a_continuous_earned_game_mechanic_not_an_opening_reward(self):
        box = CONTRACT["cardboardBoxContract"]
        self.assertEqual(box["permanentItemId"], "BOX")
        self.assertEqual(box["temporaryNonCanonPreludeId"], "FREIGHT_CARTON")
        self.assertIn("3F", box["authority"])
        self.assertFalse(box["highPressureLowPressureBeat"]["boxConsumed"])
        self.assertIn("NOT IMPLEMENTED", box["currentSourceStatus"])
        self.assertIn('FName(TEXT("BOX"))', EQUIP)
        self.assertIn('FName(TEXT("FREIGHT_CARTON"))', EQUIP)
        self.assertIn('AwardItem(TEXT("FREIGHT_CARTON"), 1)', MISSION)
        self.assertNotIn('AwardItem(TEXT("BOX"), 1)', MISSION)
        self.assertIn('RevokeTemporaryItem(TEXT("FREIGHT_CARTON"))', MISSION)

    def test_box_disguise_controls_reused_by_an_eventually_earned_standard_box(self):
        self.assertIn('IsItemEquipped(TEXT("BOX"))', PLAYER)
        self.assertIn('IsItemEquipped(TEXT("FREIGHT_CARTON"))', PLAYER)
        self.assertIn('FREIGHT CARTON [TEMPORARY]', WEB)
        self.assertIn('CARDBOARD BOX [F3 / CARD 2]', WEB)
        self.assertIn("canonicalBoxSlot.usable=false", WEB)
        self.assertIn("boxSlot.usable=stage<0&&world.box", WEB)

    def test_standard_guard_box_inspection_is_not_instant_or_omniscient(self):
        for marker in ("bLegitimateReceivingParcel", "bRealSightline",
                       "CartonInspectionSeconds", "CartonInspectionSeconds >= 1.30f",
                       "ContainerInspection", "ContainerOpened"):
            self.assertIn(marker, GUARD)
        for marker in ("!nearBelt()", "boxInspectionAge", "boxIdentified",
                       "boxInspect", "boxExposed"):
            self.assertIn(marker, WEB)

    def test_freight_ambient_scene_advances_without_forcing_cutscene(self):
        self.assertIn("if(riding)movePlayer(", WEB)
        self.assertIn("player.x>=-22.25", WEB)
        self.assertIn("say('freightShift')", WEB)
        self.assertIn("freightAmbientHeard:world.freightAmbientHeard", WEB)
        self.assertIn("'freightAmbientHeard'", WEB)
        self.assertIn("RECEIVING CLERK (OFF)", LINES)
        self.assertIn("Player->AddActorWorldOffset(", MISSION)
        self.assertIn("if (!bHeardFreightClerk && P.X >= -2570.f)", MISSION)
        self.assertIn("Save->bHeardFreightClerk = bHeardFreightClerk;", MISSION)
        self.assertIn("bHeardFreightClerk = Save->bHeardFreightClerk;", MISSION)
        self.assertIn("bool bHeardFreightClerk = false", SAVE)

    def test_side_game_inspiration_is_not_promoted_to_original_canon(self):
        by_id = {entry["id"]:entry for entry in INFLUENCES["adaptations"]}
        self.assertIn("BOX_TRANSIT_TENSION_RELEASE", by_id)
        self.assertIn("one_source_prototype", by_id["BOX_TRANSIT_TENSION_RELEASE"]["status"])
        self.assertIn("1987", by_id["BOX_TRANSIT_TENSION_RELEASE"]["source_canon"])
        self.assertTrue(any("reusable" in x for x in by_id["CARDBOARD_CONCEALED_BOX"]["verification"]))

if __name__ == "__main__":
    unittest.main()
