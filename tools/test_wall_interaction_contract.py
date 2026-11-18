"""Static source-contract checks, not a substitute for an Unreal build or playtest."""
from pathlib import Path
import unittest
ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "unreal/MGRemaster/Source/MGRemaster/Private"
PLAYER = (SRC / "MGPlayerCharacter.cpp").read_text(encoding="utf-8")
MISSION = (SRC / "MGGrayFoxMission.cpp").read_text(encoding="utf-8")
GUARD = (SRC / "MGGuardActor.cpp").read_text(encoding="utf-8")

class WallInteractionContract(unittest.TestCase):
    def test_breachable_wall_tag(self):
        self.assertIn('StartsWith(TEXT("WeakWall_"))', MISSION)
        self.assertIn('Actor->Tags.Add(TEXT("MG_Breachable"))', MISSION)
        self.assertIn('ActorHasTag(TEXT("MG_Breachable"))', PLAYER)

    def test_breach_priority(self):
        segment = PLAYER.split("void AMGPlayerCharacter::Interact()", 1)[1].split(
            "void AMGPlayerCharacter::ToggleMissionLog()", 1)[0]
        self.assertLess(segment.index("Mission->TryInteract(this)"),
                        segment.index("TryEnterCover()"))
        self.assertIn("Changed & (4u | 16u)", segment)

    def test_contextual_knock_event(self):
        segment = PLAYER.split("void AMGPlayerCharacter::MeleeAttack()", 1)[1]
        self.assertIn("if (bInCover) { KnockOnWall(); return; }", segment)
        knock = PLAYER.split("void AMGPlayerCharacter::KnockOnWall()", 1)[1].split(
            "void AMGPlayerCharacter::ToggleCover()", 1)[0]
        self.assertIn("Hit.ImpactPoint, CoverNormal, HearingRadius", knock)
        self.assertNotIn("LastKnownPlayerPosition", knock)

    def test_guard_can_investigate_and_return(self):
        for phase in ["React", "Approach", "Search", "Return"]:
            self.assertIn("EDistractionPhase::" + phase, GUARD)
        self.assertIn("if (AwarenessState >= 2)", GUARD)
        self.assertIn("SetActorLocation(GetActorLocation() + Step, true)", GUARD)

if __name__ == "__main__":
    unittest.main()