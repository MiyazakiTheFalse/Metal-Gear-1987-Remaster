"""Source-only checks: optional radio must preserve agency and prison progression."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "unreal/MGRemaster/Source/MGRemaster/Private"
PLAYER = (SOURCE / "MGPlayerCharacter.cpp").read_text(encoding="utf-8")
MISSION = (SOURCE / "MGGrayFoxMission.cpp").read_text(encoding="utf-8")
HUD = (SOURCE / "MGPrototypeHUD.cpp").read_text(encoding="utf-8")

class OptionalTransceiverContract(unittest.TestCase):
    def test_transceiver_not_confiscated_or_mission_gated(self):
        body = PLAYER.split("void AMGPlayerCharacter::ToggleCodec()", 1)[1].split(
            "void AMGPlayerCharacter::CycleGraphicsQuality()", 1)[0]
        self.assertNotIn("SetGamePaused", body)
        self.assertNotIn("MG_FOX_RESCUED", body)
        self.assertNotIn("IsConfiscated", body)
        self.assertIn("bCodecOpen = true;", body)

    def test_radio_contact_is_optional(self):
        body = MISSION.split("bool AMGGrayFoxMission::HasIncomingTransceiverPrompt()", 1)[1].split(
            "FString AMGGrayFoxMission::GetMissionLogText()", 1)[0] if (
                "FString AMGGrayFoxMission::GetMissionLogText()" in MISSION.split(
                "bool AMGGrayFoxMission::HasIncomingTransceiverPrompt()", 1)[1]) else MISSION.split(
                "bool AMGGrayFoxMission::HasIncomingTransceiverPrompt()", 1)[1]
        self.assertIn("GetHintLevel() >= 1", body)
        self.assertIn("bUnderAlert", body)
        self.assertIn("GetTransceiverDialogue", MISSION)

    def test_schneider_does_not_know_prison_plan(self):
        self.assertIn("I never saw", MISSION)
        self.assertIn("the prison plans", MISSION)
        self.assertIn("BIG BOSS:", MISSION)

    def test_hud_renders_transcript_not_a_control_lock(self):
        self.assertIn("Transcript.ParseIntoArrayLines", HUD)
        self.assertIn("MOVE: CONTINUE", HUD)
        self.assertIn("HasIncomingTransceiverPrompt()", HUD)

    def test_early_objective_does_not_spoil_puzzle(self):
        self.assertIn("Search your cell for a way out.", MISSION)
        self.assertNotIn("Find a weakness in the cell wall.", MISSION)

if __name__ == "__main__":
    unittest.main()
