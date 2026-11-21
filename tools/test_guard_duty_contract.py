"""Data-first checks for purposeful fictional guard posts and the prison census.

These are source checks, not a navigation-mesh bake, Unreal compilation, or
proof of exact Metal Gear 1987 / J2ME enemy and prisoner counts.
Run: python -m unittest tools/test_guard_duty_contract.py
"""
import json
import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = json.loads((ROOT / "data/1995_guard_duty_and_prison_occupancy_contract.json").read_text(encoding="utf-8"))
MISSIONS = json.loads((ROOT / "data/1995_numbered_missions.json").read_text(encoding="utf-8"))
OPENING = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGMissionOne.cpp").read_text(encoding="utf-8")
PRISON = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGGrayFoxMission.cpp").read_text(encoding="utf-8")
GUARD = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGGuardActor.cpp").read_text(encoding="utf-8")
GUARD_H = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Public/MGGuardActor.h").read_text(encoding="utf-8")

class GuardDutyContractTests(unittest.TestCase):
    def test_one_job_per_current_guard(self):
        current = CONTRACT["current_implemented_posts"]
        ids = [g["id"] for g in current]
        self.assertEqual(9, len(ids))
        self.assertEqual(len(ids), len(set(ids)))
        for guard in current:
            for key in ("duty", "protected_asset", "purpose", "response", "patrol"):
                self.assertTrue(guard.get(key), (guard["id"], key))
            self.assertTrue(guard["implemented"])
        self.assertEqual(2, sum(g["mission"] == 2 for g in current))

    def test_opening_guards_have_explicit_duties_not_random_coordinates(self):
        ids = [x["id"] for x in CONTRACT["current_implemented_posts"] if x["mission"] == 1]
        self.assertEqual(7, len(ids))
        for name in ids:
            self.assertIn(f'TEXT("{name}"), FName(TEXT(', OPENING)
        self.assertIn('Guard->ConfigureDuty(FName(Route.Name)', OPENING)
        self.assertIn('Guard->ConfigurePatrol(Route.Nodes)', OPENING)
        self.assertIn('Route.AssetFocus', OPENING)
        self.assertNotIn('TEXT("G06_OptionalMaintenance")', OPENING)

    def test_detention_guard_jobs_and_visual_detainee_count(self):
        occupancy = CONTRACT["prison"]
        self.assertEqual(4, occupancy["count_visible_detainees"])
        self.assertEqual(len(occupancy["people"]), occupancy["count_visible_detainees"])
        self.assertEqual(2, sum(x["canonical"] for x in occupancy["people"]))
        self.assertEqual(2, sum(not x["canonical"] for x in occupancy["people"]))
        for prisoner in occupancy["people"]:
            if prisoner.get("source_status") == "remake_optional_noncanonical":
                self.assertFalse(prisoner["counts_as_pow_rescue"])
                self.assertFalse(prisoner["interactive"])
                self.assertIn(prisoner["id"], PRISON)
        for duty in ("CELL_BLOCK_WATCH", "PROPERTY_CUSTODY"):
            self.assertIn(f'FName(TEXT("{duty}"))', PRISON)
        self.assertEqual(2, PRISON.count("GetWorld()->SpawnActor<AMGGuardActor>"))

    def test_background_cells_have_colliding_containment(self):
        # A visual-only cell must not accidentally be physically accessible
        # through an upright or crouched mesh gap just because the floor moved.
        self.assertIn("HoldingWingBarsUpper", PRISON)
        self.assertIn("HoldingWingBarsLower", PRISON)
        self.assertIn("BarX <= 1275.f; BarX += 50.f", PRISON)
        self.assertIn("FVector(.12f,.14f,3.8f), true", PRISON)
        self.assertIn("AMBIENT_DETAINEE_A_UNVERIFIED", PRISON)
        self.assertIn("AMBIENT_DETAINEE_B_UNVERIFIED", PRISON)

    def test_guard_ai_has_work_and_search_without_floor_freezing(self):
        self.assertIn("void AMGGuardActor::ConfigureDuty(", GUARD)
        self.assertIn("void AMGGuardActor::AdvanceAssignedRoutine(", GUARD)
        self.assertIn("DutyFocusPoint - GetActorLocation()", GUARD)
        self.assertIn("AssignmentAnchor", GUARD)
        self.assertIn("AssignedDutyId.IsNone()", GUARD)
        self.assertIn("AdvanceAssignedRoutine(DeltaSeconds);", GUARD)
        self.assertIn("FMath::Abs(GetActorLocation().Z - SoundLocation.Z) > 260.f", GUARD)
        self.assertIn("ConfigureDuty(FName DutyId", GUARD_H)

    def test_policy_covers_entire_numbered_campaign_without_fake_guard_counts(self):
        mission_ids = {m["mission"] for m in CONTRACT["mission_authoring"]}
        self.assertEqual(set(range(1, 9)), mission_ids)
        self.assertEqual(8, MISSIONS["mission_count"])
        for m in CONTRACT["mission_authoring"]:
            self.assertTrue(m["required_posts"])
            self.assertIn("planned_counts", m)
        self.assertTrue(all("additional" in m["planned_counts"]
                            for m in CONTRACT["mission_authoring"][2:]))

if __name__ == "__main__":
    unittest.main()
