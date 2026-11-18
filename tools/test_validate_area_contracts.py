"""Doctrine 005 spatial contract tests are structural, NOT a UE playtest."""
import copy
import json
import sys
import unittest
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from validate_area_contracts import REGISTRY, BEAT_REGISTRY, validate

class AreaContractsTests(unittest.TestCase):
    def setUp(self):
        self.data = json.loads(REGISTRY.read_text(encoding="utf-8"))
        beats = json.loads(BEAT_REGISTRY.read_text(encoding="utf-8"))
        self.beat_ids = {b["id"] for b in beats["beats"]}

    def test_all_seed_areas_complete(self):
        self.assertEqual(validate(self.data, self.beat_ids), [])

    def test_broken_beat_reference_rejected(self):
        modified = copy.deepcopy(self.data)
        modified["areas"][0]["beat_ids"].append("unregistered.event")
        self.assertTrue(any("missing beat" in p for p in validate(modified, self.beat_ids)))

    def test_missing_sound_occlusion_rejected(self):
        modified = copy.deepcopy(self.data)
        del modified["areas"][0]["audio"]["occlusion"]
        self.assertTrue(any("audio.occlusion" in p for p in validate(modified, self.beat_ids)))

    def test_unjustified_lock_rejected(self):
        modified = copy.deepcopy(self.data)
        modified["areas"][1]["timing"]["lock_seconds"] = 3
        self.assertTrue(any("agency" in p for p in validate(modified, self.beat_ids)))

    def test_color_dependency_rejected(self):
        modified = copy.deepcopy(self.data)
        modified["areas"][2]["accessibility"]["color_independent"] = False
        self.assertTrue(any("colour" in p for p in validate(modified, self.beat_ids)))

if __name__ == "__main__":
    unittest.main()
