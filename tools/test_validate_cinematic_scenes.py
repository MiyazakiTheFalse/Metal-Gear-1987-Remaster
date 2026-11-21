import copy
import json
import unittest
from validate_cinematic_scenes import validate, ROOT

class CinematicSourceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.registry=json.loads((ROOT/"data/cinematic_scene_contracts.json").read_text())
    def test_source_design_contract(self):
        self.assertEqual(validate(self.registry), [])
    def test_missing_sequence_fails(self):
        data=copy.deepcopy(self.registry)
        data["scenes"]=[s for s in data["scenes"] if s["id"]!="shotgunner_intro"]
        self.assertTrue(any("missing required scenes" in e for e in validate(data)))
    def test_faked_render_fails(self):
        data=copy.deepcopy(self.registry)
        data["scenes"][0]["status"]="UE5_approved"
        self.assertTrue(any("falsely marked produced" in e for e in validate(data)))
    def test_skipped_mandatory_intel_fails(self):
        data=copy.deepcopy(self.registry)
        data["scenes"][1]["skip"]=""
        self.assertTrue(any("missing skip" in e for e in validate(data)))
if __name__=="__main__":
    unittest.main()
