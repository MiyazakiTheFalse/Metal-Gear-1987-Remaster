"""Event-ledger provenance gate tests."""
import copy
import json
from pathlib import Path
import unittest

from validate_event_ledger import validate

ROOT = Path(__file__).resolve().parents[1]


class EventLedgerChecks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.ledger = json.loads((ROOT / "data/event_ledger.json").read_text(encoding="utf-8"))

    def test_candidate_records_are_valid(self):
        self.assertEqual(validate(self.ledger), [])

    def test_fake_j2me_verification_fails(self):
        altered = copy.deepcopy(self.ledger)
        altered["entries"][0]["evidence_status"] = "j2me_verified"
        self.assertTrue(any("J2ME verification requires" in error for error in validate(altered)))

    def test_missing_interruption_policy_fails(self):
        altered = copy.deepcopy(self.ledger)
        altered["entries"][0]["interruption_policy"] = ""
        self.assertTrue(any("interruption" in error for error in validate(altered)))

    def test_duplicate_id_fails(self):
        altered = copy.deepcopy(self.ledger)
        altered["entries"][1]["id"] = altered["entries"][0]["id"]
        self.assertTrue(any("repeated" in error for error in validate(altered)))

    def test_misplaced_input_lock_fails(self):
        altered = copy.deepcopy(self.ledger)
        altered["entries"][0]["control_policy"] = "brief_lock_with_safe_restore"
        self.assertTrue(any("control lock" in error for error in validate(altered)))


if __name__ == "__main__":
    unittest.main()
