"""Source contract for paced-but-optional opening dialogue and presentation events."""
import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PLAYER=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Private/MGPlayerCharacter.cpp").read_text()
HUD=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Private/MGPrototypeHUD.cpp").read_text()
MISSION=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Private/MGGrayFoxMission.cpp").read_text()
HEADER=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Public/MGGrayFoxMission.h").read_text()
RUST=(ROOT/"rust/src/pacing.rs").read_text()

class PacingIntegrationSource(unittest.TestCase):
    def test_incoming_call_stages_visible_lines_without_lock(self):
        self.assertIn("GetVisibleRadioLineCount", PLAYER)
        self.assertIn("RadioTranscriptSeconds", PLAYER)
        self.assertIn("Player->GetVisibleRadioLineCount(Lines.Num())", HUD)
        self.assertIn("bStagedOpeningCall = false;",PLAYER)
        self.assertIn("bStagedOpeningCall = true;",PLAYER)
        self.assertIn("CloseEquipmentWindows();",PLAYER)
    def test_phases_align_across_fallback_and_rust(self):
        for name,number in {"DETENTION_APPROACH":9,"AFTERMATH_SEARCH":10,"CHAPTER_EXIT":11}.items():
            self.assertIn(f"pub const {name}: u32 = {number};",RUST)
            self.assertIn(f"case {number}: return",MISSION)
        self.assertIn("MG_FOX_CARD3_RECOVERED",MISSION)
        self.assertIn("MG_FOX_SHOTMAKER_DEFEATED",MISSION)
    def test_beat_events_only_observe_authoritative_state(self):
        self.assertIn("OnPacingBeatChanged",HEADER)
        self.assertIn("NotifyPacingPhaseIfChanged()",MISSION)
        self.assertIn("GetSuggestedBeatIntensity",MISSION)
        self.assertIn("LastPresentedPacingPhase = NewPhase",MISSION)
        self.assertIn("Report.target_intensity",MISSION)
    def test_boss_victory_enables_nonspoiling_hints(self):
        self.assertIn("if (bUnderAlert || CurrentPhase == 8 || CurrentPhase == 5",MISSION)
        self.assertIn("Investigate the secured storeroom",MISSION)
        self.assertIn("Use Card 2 at the northern storeroom",MISSION)

if __name__=="__main__": unittest.main()
