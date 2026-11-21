"""Small regression suite for the 1995 narrative registry."""
import json
import unittest
from pathlib import Path
from validate_character_arcs import validate

ROOT = Path(__file__).resolve().parents[1]
DATA = json.loads((ROOT / "data/character_arc_registry.json").read_text(encoding="utf-8"))


class CharacterStoryContractTests(unittest.TestCase):
    def test_arc_link_integrity(self):
        self.assertEqual([], validate(DATA))

    def test_core_reveal_order(self):
        stages = {s["id"]: i for i, s in enumerate(DATA["stages"])}
        beats = {b["id"]: b for b in DATA["beats"]}
        self.assertLess(stages[beats["fox.revelation"]["stage"]],
                        stages[beats["madnar.weakness"]["stage"]])
        self.assertLess(stages[beats["madnar.weakness"]["stage"]],
                        stages[beats["boss.reveal"]["stage"]])
        self.assertIn("ellen.rescue", beats["madnar.reunion_confirmation"]["depends_on"])

    def test_important_spoilers_not_in_opening(self):
        beats = {b["id"]: b for b in DATA["beats"]}
        self.assertNotEqual("S01", beats["boss.reveal"]["stage"])
        self.assertNotEqual("S01", beats["boss.abort_order"]["stage"])

    def test_optional_radio_is_not_a_hard_gate(self):
        self.assertTrue(DATA["gate_policy"]["optional_radio"].startswith("Never"))
        self.assertIn("not", DATA["dependency_semantics"].lower())
        self.assertTrue(all(b["provenance"] != "j2me_verified" for b in DATA["beats"]))

    def test_gray_fox_story_stays_in_1995(self):
        fox = next(c for c in DATA["characters"] if c["id"] == "gray_fox")
        self.assertEqual(4, len(fox["beats"]))
        self.assertIn("cyborg", fox["story_boundary"].lower())
        self.assertNotIn("fox.second_boss", fox["beats"])

    def test_unique_named_bosses_have_separate_encounters(self):
        characters = {c["id"]: c for c in DATA["characters"]}
        for cid in ["shoot_gunner","machinegun_kid","fire_trooper","coward_duck"]:
            self.assertEqual(1, len(characters[cid]["beats"]))


if __name__ == "__main__":
    unittest.main()
