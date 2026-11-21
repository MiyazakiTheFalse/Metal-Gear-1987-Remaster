import copy
import json
from pathlib import Path
import unittest
from validate_full_game_bibles import validate, ROOT

class FullGameBibleContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reg = json.loads((ROOT / "data/character_bible_registry.json").read_text())
        cls.tx = json.loads((ROOT / "data/tx55_boss_contract.json").read_text())
    def test_project_contract(self):
        self.assertEqual(validate(self.reg, self.tx), [])
    def test_missing_significant_character_fails(self):
        c = copy.deepcopy(self.reg)
        c["entries"] = [e for e in c["entries"] if e["id"] != "VENOM_SNAKE"]
        c["count"] -= 1
        self.assertTrue(any("VENOM_SNAKE" in e for e in validate(c, self.tx)))
    def test_faked_verification_fails(self):
        c = copy.deepcopy(self.reg)
        c["entries"][0]["status"] = "shipped"
        self.assertTrue(any("falsely verified" in e for e in validate(c, self.tx)))
    def test_damaged_tx55_sequence_fails(self):
        t = copy.deepcopy(self.tx)
        t["fixed_order"][-1] = "L"
        self.assertTrue(any("order mismatch" in e for e in validate(self.reg, t)))
    def test_hidden_final_step_cannot_be_leaked(self):
        t = copy.deepcopy(self.tx)
        t["known_to_player"] = 16
        self.assertTrue(any("direction puzzle" in e for e in validate(self.reg, t)))

if __name__ == "__main__":
    unittest.main()
