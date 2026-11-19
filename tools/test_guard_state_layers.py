"""Source-only guard-state contract checks, not a UE5.7 compile or behaviour test."""
import json
import unittest
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
HEADER=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Public/MGGuardActor.h").read_text()
GUARD=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Private/MGGuardActor.cpp").read_text()
MHEAD=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Public/MGGrayFoxMission.h").read_text()
MISSION=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Private/MGGrayFoxMission.cpp").read_text()
HUD=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Private/MGPrototypeHUD.cpp").read_text()
CONTRACT=json.loads((ROOT/"data/guard_ai_behavior_contract.json").read_text())

class GuardStateLayerSourceTests(unittest.TestCase):
    def test_separate_duty_awareness_evidence_and_action(self):
        for enum in ("EMGGuardRole","EMGGuardAwarenessPhase",
                     "EMGGuardEvidenceKind","EMGGuardPosture"):
            self.assertIn("enum class "+enum,HEADER)
        self.assertIn("enum class EMGFacilitySecurityState",MHEAD)
        self.assertEqual(len(CONTRACT["layers"]),5)
        self.assertIn("OnAwarenessPhaseChanged",HEADER)
        self.assertIn("OnPostureChanged",HEADER)
        self.assertIn("OnFacilitySecurityStateChanged",MHEAD)

    def test_direct_visual_proof_required_for_damage(self):
        self.assertIn("bConfirmedVisualNow && AwarenessState >= 3",GUARD)
        self.assertIn("VisualConfirmationSeconds >= .16f",GUARD)
        self.assertIn("AttackWindupSeconds = .38f",GUARD)
        self.assertIn("else AttackWindupSeconds = 0.f",GUARD)
        self.assertIn("OnMeleeWarning()",GUARD)
        self.assertIn("OnMeleeImpact()",GUARD)

    def test_sound_body_and_radio_are_different_evidence(self):
        for t in ("StructuralImpact", "DiscoveredBody", "SquadBodyReport",
                  "ObservedIntruder", "HeardMovement", "SquadSighting"):
            self.assertIn("EMGGuardEvidenceKind::"+t,GUARD)
        self.assertIn("PendingConfirmedRadioSighting = Player->GetActorLocation()",GUARD)
        self.assertIn("ReceiveLocalSquadReport(PendingConfirmedRadioSighting, this)",GUARD)
        self.assertIn("NotifyGuardBodyReported(BodyReportLocation)",GUARD)
        self.assertIn("NotifyGuardContactReported(PendingConfirmedRadioSighting)",GUARD)

    def test_facility_caution_does_not_mean_instant_attack(self):
        for status in ("Normal","Caution","Alert","Evasion","StandDown"):
            self.assertIn(status, MHEAD)
        self.assertIn("FacilityAlertRemaining = FMath::Max",MISSION)
        self.assertIn("FacilityEvasionRemaining",MISSION)
        self.assertIn("FacilityStandDownRemaining",MISSION)
        self.assertIn("!bFacilityLockdown",GUARD)
        self.assertIn("bFacilityCaution",GUARD)
        self.assertIn("bUnderAlert = bLocalConfirmedContact",MISSION)

    def test_patrol_axis_and_physical_world_evidence(self):
        self.assertIn("FVector::DotProduct(\n        GetActorLocation() - PatrolOrigin, PatrolAxis)",GUARD)
        self.assertIn("ActualGroundSpeedCmPerSec",GUARD)
        self.assertIn("HearWallKnock(Location",MISSION)
        self.assertIn("OnSurfaceImpact(",MISSION)
        self.assertIn("SetActorLocation(Before + SmoothedGroundVelocity * DeltaSeconds, true)",GUARD)
        self.assertIn("bConfirmedVisualThisFrame && AwarenessState >= 3",GUARD)

    def test_infiltration_map_and_false_alert_not_mislabeled(self):
        self.assertIn("const bool bPrelude = Player->GetActorLocation().X < -1250.f",HUD)
        self.assertIn("HasDirectVisualContact()",HUD)
        self.assertIn("GetFacilitySecurityState()",HUD)
        self.assertIn("EMGGuardAwarenessPhase::Searching",HUD)

    def test_not_falsely_marked_tested(self):
        self.assertFalse(CONTRACT["approved_in_unreal"])
        self.assertEqual(CONTRACT["status"],"committed_source_uncompiled")
        self.assertGreaterEqual(len(CONTRACT["known_gaps"]),5)

if __name__=="__main__":unittest.main()
