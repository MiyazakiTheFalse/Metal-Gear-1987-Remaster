"""Original 1987 chronology and safe connected 3D post-Card-3 source checks.

Run locally: python -m unittest discover -s tools -p 'test_*.py'
Unreal compilation, physical reachability, navigation and player reception are NOT
proven by this static test. Original map is still awaiting direct edition extraction.
"""
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CPP = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGGrayFoxMission.cpp").read_text()
HEADER = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Public/MGGrayFoxMission.h").read_text()
DOG_H = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Public/MGSentryDogActor.h").read_text()
DOG = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGSentryDogActor.cpp").read_text()
PLAYER = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGPlayerCharacter.cpp").read_text()
PROVENANCE = json.loads((ROOT / "data/MSX2_1987_STORY_AND_WEAPON_PROVENANCE.json").read_text())
BEATS = json.loads((ROOT / "data/beat_contracts.json").read_text())
AREAS = json.loads((ROOT / "data/area_contracts.json").read_text())

class PostCard3SourceChecks(unittest.TestCase):
    def test_source_priority_and_weapon_acquisition_order(self):
        policy = PROVENANCE["source_policy"]
        self.assertIn("1987 MSX2", policy["priority"][0])
        self.assertIn("REMAKE_INTERPRETATION", policy["translation_rule"])
        self.assertEqual([x["id"] for x in PROVENANCE["chronology"]],
            ["b1f1_infiltration", "prison_and_gray_fox", "shotmaker", "after_card3"])
        self.assertEqual(PROVENANCE["weapon_rules"]["original_mine_limit"], 3)
        self.assertEqual(PROVENANCE["weapon_rules"]["original_plastic_explosive_limit"], 1)
        self.assertEqual(PROVENANCE["weapon_rules"]["noncanonical_debug_only"], ["STUN"])
        self.assertTrue(any("MAC11" in x for x in PROVENANCE["weapon_rules"]["smg_reference"].splitlines()))

    def test_gate_is_full_width_and_later_walkway_is_connected(self):
        self.assertIn('TEXT("ChapterExit"), FVector(3610.f, 0.f, 90.f)', CPP)
        self.assertIn('FVector(.45f, 10.f, 3.8f)', CPP) # Full-width gate, no skirting.
        self.assertIn('TEXT("BossEastBoundaryNorth")', CPP)
        self.assertIn('TEXT("BossEastBoundarySouth")', CPP)
        self.assertNotIn('TEXT("BossEastBoundary"),', CPP) # Old full dead-end removed.
        self.assertIn("CreatePostCard3Service();", CPP)
        self.assertIn("void AMGGrayFoxMission::CreatePostCard3Service()", CPP)
        self.assertIn('TEXT("PostCard3Floor")', CPP)
        self.assertIn('TEXT("PostCard3FutureSectorWall")', CPP)
        self.assertIn("if (IsValid(ChapterExit)) ChapterExit->Destroy()", CPP)

    def test_prone_duct_has_physical_clearance_not_a_prompt_warp(self):
        for name in ("ServiceDuctRoof", "ServiceDuctSouthRail", "ServiceDuctNorthRail", "ServiceDuctExitGrate"):
            self.assertIn('TEXT("' + name + '")', CPP)
        self.assertIn('FVector(4430.f, -300.f, 10.f)', CPP)
        self.assertIn('FVector(3.3f, 2.4f, .58f)', CPP)
        self.assertIn('const float ProneHalf = 38.f;', PLAYER)
        self.assertIn('GetCharacterMovement()->CrouchedHalfHeight = 55.f;', PLAYER)
        self.assertIn("OverlapBlockingTestByChannel", PLAYER)
        self.assertIn("SweepSingleByChannel", PLAYER)
        self.assertNotIn("TeleportTo(", CPP)
        self.assertIn("not an extracted J2ME or MSX2 map", CPP)

    def test_canine_windup_uses_real_los_and_does_not_punch_through_cover(self):
        for s in ("CanObserve(", "LineTraceSingleByChannel", "Hit.GetActor() == Player",
                  "EMGSentryDogState::Lunge", "LungeRemaining = .36f",
                  "EMGSentryDogState::Recover", "GetActorLocation()", "OnSentryBark()",
                  "ReceiveEnvironmentalReport(GetActorLocation(), this)"):
            self.assertIn(s, DOG)
        self.assertIn("DogA->ConfigureBeatPatrol", CPP)
        self.assertIn("DogB->ConfigureBeatPatrol", CPP)
        self.assertIn("OnSentryLungeTelegraph", DOG_H)
        self.assertNotIn("ApplyDamage(Player, 15.f", CPP)

    def test_new_sector_checkpoint_only_allowed_after_canonical_card3(self):
        self.assertIn("MG_FOX_SLICE_COMPLETED", CPP)
        self.assertIn("(F & MG_FOX_SLICE_COMPLETED) ? 5480.f", CPP)
        self.assertIn("bPostCard3ServiceEntered", CPP)
        self.assertIn("SaveProgress(Player);", CPP)
        self.assertIn('SECURITY KENNEL AHEAD', CPP)
        self.assertIn("bPostCard3ServiceEntered = bHasLoadedSave", CPP)
        self.assertIn("bPostCard3ServiceEntered = false;", HEADER)

    def test_doctrine_contracts_separate_existing_geometry_from_future_work(self):
        beat_ids = {b["id"] for b in BEATS["beats"]}
        area_ids = {a["id"] for a in AREAS["areas"]}
        for b in ("building1.after_card3_kennel", "building1.after_card3_crawl",
                  "building1.after_card3_explosive_wall", "building1.machinegun_kid_leadin"):
            self.assertIn(b, beat_ids)
        for a in ("building1.after_card3_dog_corridor", "building1.after_card3_crawl",
                  "building1.after_card3_explosive_boundary"):
            self.assertIn(a, area_ids)
        future = next(b for b in BEATS["beats"] if b["id"] == "building1.after_card3_explosive_wall")
        self.assertEqual(future["implementation_status"], "design_only_not_playable")
        self.assertEqual(future["state"]["transaction"].split(";")[0], "No new mission action yet")
        self.assertTrue(all(b.get("verification_status") == "unverified" for b in BEATS["beats"][-4:]))

if __name__ == "__main__":
    unittest.main()
