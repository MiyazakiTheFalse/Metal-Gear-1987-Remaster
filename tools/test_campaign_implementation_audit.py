import sys
import unittest
from copy import deepcopy
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from audit_campaign_implementation import audit


def fixture():
    return {
        "dimensions": ["browser_runtime", "unreal_runtime", "player_playtest"],
        "missions": [{"id": "P01", "title": "Test", "beats": [{
            "id": "P01.first", "ordinal": 1, "overall": "source_prototype",
            "completeness": {"browser_runtime": "source_prototype",
                             "unreal_runtime": "source_prototype",
                             "player_playtest": "not_started"}}]}]
    }


class CampaignAuditTests(unittest.TestCase):
    def test_source_does_not_count_as_running(self):
        errors, report = audit(fixture())
        self.assertEqual(errors, [])
        self.assertEqual(report["overall_statuses"], {"source_prototype": 1})
        self.assertEqual(report["runtime_mismatch_count"], 0)

    def test_catches_unsynchronised_builds_without_fabricating_failure(self):
        data = fixture()
        data["missions"][0]["beats"][0]["completeness"]["unreal_runtime"] = "design_only"
        errors, report = audit(data)
        self.assertEqual(errors, [])
        self.assertEqual(report["runtime_status_mismatches"][0]["beat"], "P01.first")

    def test_rejects_unverified_completion(self):
        data = fixture()
        data["missions"][0]["beats"][0]["overall"] = "production_ready"
        errors, _ = audit(data)
        self.assertTrue(any("without player test evidence" in e for e in errors))
        self.assertTrue(any("production ready requires" in e for e in errors))

    def test_catches_duplicate_and_missing_fields(self):
        data = fixture()
        copy = deepcopy(data["missions"][0]["beats"][0])
        copy["ordinal"] = 1
        del copy["completeness"]["unreal_runtime"]
        data["missions"][0]["beats"].append(copy)
        errors, _ = audit(data)
        self.assertTrue(any("duplicate beat id" in e for e in errors))
        self.assertTrue(any("duplicate ordinal" in e for e in errors))
        self.assertTrue(any("missing dimension unreal_runtime" in e for e in errors))


if __name__ == "__main__":
    unittest.main()
