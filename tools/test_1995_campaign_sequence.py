"""Narrative chronology regression tests; cannot substitute for source-game trace."""
import json
import unittest
from pathlib import Path
from validate_1995_sequence import validate

ROOT = Path(__file__).resolve().parents[1]
CHRONOLOGY = json.loads((ROOT / "data/1995_campaign_sequence.json").read_text(encoding="utf-8"))
ARCS = json.loads((ROOT / "data/character_arc_registry.json").read_text(encoding="utf-8"))


class Campaign1995StoryTests(unittest.TestCase):
    def test_chronology_is_integral(self):
        self.assertEqual([], validate(CHRONOLOGY, ARCS))

    def test_from_insertion_to_escape_with_sources_held_separate(self):
        self.assertEqual(1995, CHRONOLOGY["year"])
        self.assertGreaterEqual(len(CHRONOLOGY["sequence"]), 40)
        self.assertGreaterEqual(len(ARCS["beats"]), 100)
        self.assertEqual("N00_PRELUDE", CHRONOLOGY["sequence"][0]["id"])
        self.assertEqual("N39_CLOSING_BROADCAST", CHRONOLOGY["sequence"][-1]["id"])
        self.assertTrue(all(s["grade"] != "j2me_verified"
                            for s in CHRONOLOGY["sequence"]))

    def test_ellen_rescue_is_building_one_revisit(self):
        steps = {s["id"]: s for s in CHRONOLOGY["sequence"]}
        self.assertIn("building_1", steps["N22_HIDDEN_ELLEN"]["location"])
        self.assertIn("building_2", steps["N24_RETURN_TO_SCIENTIST"]["location"])
        self.assertLess(steps["N20_ACTUAL_MADNAR"]["order"],
                        steps["N22_HIDDEN_ELLEN"]["order"])
        self.assertLess(steps["N22_HIDDEN_ELLEN"]["order"],
                        steps["N24_RETURN_TO_SCIENTIST"]["order"])

    def test_original_endgame_order(self):
        steps = {s["id"]: s for s in CHRONOLOGY["sequence"]}
        self.assertLess(steps["N35_TX55_SABOTAGE"]["order"],
                        steps["N36_SELF_DESTRUCT"]["order"])
        self.assertLess(steps["N36_SELF_DESTRUCT"]["order"],
                        steps["N37_FINAL_BIG_BOSS"]["order"])
        self.assertLess(steps["N31_BROTHER_FREED"]["order"],
                        steps["N38_ESCAPE_LEFT_LADDER"]["order"])

    def test_later_canon_not_in_initial_foxhound_briefing(self):
        steps = {s["id"]: s for s in CHRONOLOGY["sequence"]}
        self.assertNotIn("venom_snake", steps["N01_INSERTION"]["cast"])
        self.assertIn("venom_snake", steps["N37_FINAL_BIG_BOSS"]["cast"])
        self.assertTrue(all("not_established_by_source" == step["exact_1995_time"]
                            for step in CHRONOLOGY["sequence"]))

    def test_all_character_contracts_have_story_scenes(self):
        included = {character for scene in CHRONOLOGY["sequence"]
                    for character in scene["cast"]}
        for c in ARCS["characters"]:
            self.assertIn(c["id"], included)


if __name__ == "__main__":
    unittest.main()
