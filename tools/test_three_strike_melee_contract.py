"""Three-strike melee source and time-window contract.

Run: python -m unittest tools/test_three_strike_melee_contract.py
These tests validate source shape and one implementation-equivalent timing model.
They do NOT mean the Unreal project has been compiled or real montages tested.
"""
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1] / "unreal/MGRemaster/Source/MGRemaster"
PLAYER = (ROOT / "Private/MGPlayerCharacter.cpp").read_text(encoding="utf-8")
PH = (ROOT / "Public/MGPlayerCharacter.h").read_text(encoding="utf-8")
GUARD = (ROOT / "Private/MGGuardActor.cpp").read_text(encoding="utf-8")
GH = (ROOT / "Public/MGGuardActor.h").read_text(encoding="utf-8")


def read_array(name):
    match = re.search(r"constexpr float " + re.escape(name) +
                      r"\[3\]\s*=\s*\{([^}]+)\};", PLAYER)
    if not match:
        raise AssertionError(f"No 3-element authored timing array for {name}")
    result = [float(x.strip().removesuffix("f")) for x in
              match.group(1).split(",")]
    if len(result) != 3:
        raise AssertionError("Three strikes are mandatory")
    return result


class ThreeStrikeMeleeTests(unittest.TestCase):
    def test_exact_three_hits_in_order(self):
        for s in ("MeleeStrikeIndex=Strike", "Strike==1 ? LeftPunchMontage",
                  "Strike==2 ? RightPunchMontage : FinisherKickMontage",
                  "bMeleeInProgress", "MeleeQueuedInputs=0",
                  "3-MeleeStrikeIndex-MeleeQueuedInputs"):
            self.assertIn(s, PLAYER)
        self.assertIn("MeleeStrikeIndex = 0", PH)
        self.assertIn("if (Remaining>0) ++MeleeQueuedInputs;", PLAYER)
        self.assertIn("if (MeleeStrikeIndex<3 && MeleeQueuedInputs>0)", PLAYER)

    def test_damage_only_at_contact_frame(self):
        for s in ("ResolveMeleeContact();", "bMeleeHitResolved=true;",
                  "MeleePhaseSeconds>=MeleeContactStart[Beat]",
                  "MeleeDamage[MeleeStrikeIndex-1]",
                  "MeleePhaseSeconds<=MeleeContactEnd[Beat]"):
            self.assertIn(s, PLAYER)
        block = PLAYER.split("void AMGPlayerCharacter::MeleeAttack()", 1)[1].split(
            "void AMGPlayerCharacter::AdvanceMeleeCombo", 1)[0]
        self.assertNotIn("ApplyDamage", block)
        self.assertNotIn("GetTimeSeconds()", block)

    def test_distinct_contact_and_reaction_profiles(self):
        durations = read_array("MeleeDurations")
        starts = read_array("MeleeContactStart")
        ends = read_array("MeleeContactEnd")
        damage = read_array("MeleeDamage")
        self.assertEqual(3, len(durations))
        for d, lo, hi in zip(durations, starts, ends):
            self.assertTrue(0 < lo < hi < d)
        self.assertGreater(durations[2], durations[0])
        self.assertGreater(damage[2], damage[0])
        self.assertGreater(damage[2], damage[1])
        self.assertIn("ReactToMeleeStrike(MeleeStrikeIndex,Origin)", PLAYER)
        self.assertIn("MeleeKnockdownSeconds = 1.25f;", GUARD)
        self.assertIn("AddActorWorldOffset(Away*65.f,true,&Hit)", GUARD)
        self.assertIn("LastMeleeReaction==1 ? -13.f : 13.f", GUARD)
        self.assertIn("GetMeleeImpactSeconds()", GH)

    def test_visible_animation_and_skeletal_handoff(self):
        for mesh in ("ProxyLeftArm", "ProxyRightArm", "ProxyLeftLeg",
                     "ProxyRightLeg"):
            self.assertIn(mesh, PH)
            self.assertIn(mesh, PLAYER)
        for montage in ("LeftPunchMontage", "RightPunchMontage",
                        "FinisherKickMontage"):
            self.assertIn(montage, PH)
            self.assertIn(montage, PLAYER)
        self.assertIn("Anim->Montage_Play(Montage,1.f)", PLAYER)
        self.assertIn("GetMeleeStrikeAlpha()", PH)
        self.assertIn("IsMeleeHitWindow()", PH)
        self.assertIn("UpdateMeleePresentation(DeltaSeconds)", PLAYER)

    def test_geometry_and_conflicting_actions(self):
        for s in ("FMath::Abs(Delta.Z)>85.f",
                  "GetActorForwardVector().GetSafeNormal2D()",
                  "SCENE_QUERY_STAT(MGMeleeContact)",
                  "Hit.GetActor()!=Guard", "bMeleeInProgress ? 100.f",
                  "if (bMeleeInProgress) return; // No simultaneous",
                  "void AMGPlayerCharacter::FireStun()"):
            self.assertIn(s, PLAYER)
        self.assertIn("if (bInCover) { KnockOnWall(); return; }", PLAYER)
        self.assertIn("!bMeleeInProgress && !ChokedGuard.IsValid()", PLAYER)

    def test_timing_frame_rate_invariant(self):
        durations = read_array("MeleeDurations")
        starts = read_array("MeleeContactStart")
        # Simulate three inputs before the first punch completes. This is
        # precisely what the two-slot buffer is designed to accept.
        for fps in (30, 60, 120):
            beat = 0
            queued = 2
            elapsed = 0.0
            hits = []
            duration_limit = 3.0
            while beat < 3 and elapsed < duration_limit:
                t = 0.0
                had_hit = False
                while t < durations[beat] and elapsed < duration_limit:
                    t += 1.0 / fps
                    elapsed += 1.0 / fps
                    if not had_hit and t >= starts[beat]:
                        hits.append(beat + 1)
                        had_hit = True
                beat += 1 if queued else 3
                queued = max(0, queued - 1)
            self.assertEqual([1, 2, 3], hits)

    def test_no_legacy_instant_damage_combo(self):
        for old in ("MeleeComboStep", "MeleeVisualTimer",
                    "MeleeNextTime", "LastMeleeTime"):
            self.assertNotIn(old, PLAYER)
            self.assertNotIn(old, PH)


if __name__ == "__main__":
    unittest.main()
