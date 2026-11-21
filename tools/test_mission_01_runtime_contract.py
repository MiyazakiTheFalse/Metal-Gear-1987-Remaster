"""Source-only regression checks for Mission 1 Unreal/contract parity.

Run: python -m unittest discover -s tools -p 'test_*.py'
No Unreal build or gameplay assertions are implied by these tests.
"""
import json
import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DATA = json.loads((ROOT / "data/mission_01_spatial_blueprint.json").read_text(encoding="utf-8"))
RUNTIME = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGMissionOne.cpp").read_text(encoding="utf-8")
GAME_MODE = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGRemasterGameMode.cpp").read_text(encoding="utf-8")
EQUIPMENT = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGEquipmentComponent.cpp").read_text(encoding="utf-8")


class MissionOneRuntimeContractTests(unittest.TestCase):
    def test_chapter_zero_loads_mission_one_and_chapter_one_loads_prison(self):
        self.assertIn("SpawnActor<AMGMissionOne>", GAME_MODE)
        self.assertIn("SpawnActor<AMGGrayFoxMission>", GAME_MODE)
        self.assertIn("if (Chapter == 1)", GAME_MODE)
        self.assertIn("InitialiseMissionOneLoadout()", GAME_MODE)

    def test_source_pickup_ids_and_types_are_present(self):
        for pickup in DATA["pickups"]:
            expected = (
                f'AddPickup(TEXT("{pickup["id"]}"), TEXT("'
                f'{self.item_id(pickup["item"])}"),'
            )
            self.assertIn(expected, RUNTIME)
        matches = re.findall(r'AddPickup\(TEXT\("(P\d{2})"\)', RUNTIME)
        # The third-floor P07 pickup is defined in the vertical contract, not
        # the ground-floor spatial data. Do not mistakenly reject an extension.
        self.assertTrue(set(p["id"] for p in DATA["pickups"]).issubset(matches))
        self.assertEqual(len(matches), len(set(matches)))
        self.assertIn("P07", matches)

    def test_guards_are_not_duplicated_or_spawned_in_disabled_wing(self):
        for g in DATA["guards"][:5]:
            self.assertIn('TEXT("' + g["id"] + "_", RUNTIME)
        self.assertNotIn('TEXT("G06_OptionalMaintenance")', RUNTIME)

    def test_no_free_handgun_ammunition(self):
        self.assertIn('Equipment->AwardWeapon(P.Item, 0)', RUNTIME)
        self.assertIn('Entry.bOwned = false;', EQUIPMENT)
        self.assertIn('Entry.Quantity = 0;', EQUIPMENT)

    def test_card_1_key_gates_and_persistence(self):
        self.assertIn('Equipment->HasItem(TEXT("CARD1"))', RUNTIME)
        self.assertIn('SaveProgress(Player)', RUNTIME)
        self.assertIn('RestoreSavedEquipment(AMGPlayerCharacter* Player)', RUNTIME)
        self.assertIn('TEXT("MissionOneV1")', RUNTIME)

    @staticmethod
    def item_id(item):
        return {
            "ration": "RATION",
            "card_1": "CARD1",
            "binoculars": "BINOCULARS",
            "gas_mask": "GASMASK",
            "handgun": "HANDGUN",
            "land_mines": "MINES",
        }[item]


if __name__ == "__main__":
    unittest.main()
