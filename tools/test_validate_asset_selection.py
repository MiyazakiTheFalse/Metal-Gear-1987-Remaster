import copy
import json
from pathlib import Path
import unittest
from validate_asset_selection import ROOT,validate

class AssetSelectionChecks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.base=json.loads((ROOT/"data/asset_source_quality_gates.json").read_text())
    def test_current_unapproved_candidates(self):
        self.assertEqual(validate(self.base),[])
        self.assertTrue(all(not c["approved"] for c in self.base["candidates"]))
    def test_cannot_approve_unverified_asset(self):
        d=copy.deepcopy(self.base)
        d["candidates"][0]["approved"]=True
        self.assertTrue(any("approval without" in s for s in validate(d)))
    def test_weight_governance(self):
        d=copy.deepcopy(self.base)
        d["criteria_weight"]["canon_silhouette"]=900
        self.assertTrue(any("total 100" in s for s in validate(d)))

if __name__=="__main__":
    unittest.main()
