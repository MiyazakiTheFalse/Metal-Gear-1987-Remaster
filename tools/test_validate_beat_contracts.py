"""Regression tests for the global experience/temporal contract gate."""
import copy
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from validate_beat_contracts import REGISTRY, validate_registry

class IntegratedExperienceGateTests(unittest.TestCase):
    def setUp(self):
        self.registry = json.loads(REGISTRY.read_text(encoding="utf-8"))

    def test_seed_registry_has_complete_contracts(self):
        self.assertEqual(validate_registry(self.registry), [])

    def test_missing_audio_equivalent_fails(self):
        registry = copy.deepcopy(self.registry)
        del registry["beats"][0]["audio"]["non_audio_equivalent"]
        self.assertTrue(any("audio.non_audio_equivalent" in err
                            for err in validate_registry(registry)))

    def test_duplicate_event_id_fails(self):
        registry = copy.deepcopy(self.registry)
        registry["beats"].append(copy.deepcopy(registry["beats"][0]))
        self.assertTrue(any("duplicate id" in err for err in validate_registry(registry)))

    def test_forced_lock_requires_justification(self):
        registry = copy.deepcopy(self.registry)
        registry["beats"][0]["timing"]["max_forced_lock_seconds"] = 2.0
        self.assertTrue(any("control loss" in err for err in validate_registry(registry)))

    def test_bad_hint_order_fails(self):
        registry = copy.deepcopy(self.registry)
        registry["beats"][0]["timing"]["optional_hint_seconds"] = [35, 14]
        self.assertTrue(any("optional hints" in err for err in validate_registry(registry)))

    def test_false_shipping_claim_fails(self):
        registry = copy.deepcopy(self.registry)
        registry["beats"][0]["implementation_status"] = "shipped"
        self.assertTrue(any("shipped requires" in err for err in validate_registry(registry)))

    def test_test_claim_requires_evidence(self):
        registry = copy.deepcopy(self.registry)
        registry["beats"][0]["verification_status"] = "engine_tested"
        self.assertTrue(any("concrete test_evidence" in err
                            for err in validate_registry(registry)))

if __name__ == "__main__":
    unittest.main()
