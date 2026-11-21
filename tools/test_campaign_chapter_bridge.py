"""Static chapter-bridge integration regressions (not an Unreal compilation)."""
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = json.loads((ROOT / "data/1995_runtime_chapter_contract.json").read_text(encoding="utf-8"))

def read(rel):
    return (ROOT / rel).read_text(encoding="utf-8")

GAME = read("unreal/MGRemaster/Source/MGRemaster/Private/MGRemasterGameMode.cpp")
MISSION = read("unreal/MGRemaster/Source/MGRemaster/Private/MGMissionOne.cpp")
PRISON = read("unreal/MGRemaster/Source/MGRemaster/Private/MGGrayFoxMission.cpp")
PRISON_HEADER = read("unreal/MGRemaster/Source/MGRemaster/Public/MGGrayFoxSaveGame.h")
STORY = read("unreal/MGRemaster/Source/MGRemaster/Private/MGCampaignState.cpp")
STORY_HEADER = read("unreal/MGRemaster/Source/MGRemaster/Public/MGCampaignSaveGame.h")
HUD = read("unreal/MGRemaster/Source/MGRemaster/Private/MGPrototypeHUD.cpp")
PLAYER = read("unreal/MGRemaster/Source/MGRemaster/Private/MGPlayerCharacter.cpp")
RUST = read("rust/src/campaign.rs")
RUST_LIB = read("rust/src/lib.rs")


class ChapterBridgeSourceTests(unittest.TestCase):
    def test_exactly_three_campaign_chapters(self):
        self.assertEqual([0, 1, 2],
                         [chapter["number"] for chapter in CONTRACT["chapters"]])
        self.assertIn("if (Chapter == 1)", GAME)
        self.assertIn("SpawnActor<AMGGrayFoxMission>", GAME)
        self.assertIn("SpawnActor<AMGMissionOne>", GAME)

    def test_story_progression_requires_prisoner_then_rescue_and_escape(self):
        self.assertIn('HasBeat(TEXT("snake.early_pow"))', STORY)
        for label in ('"fox.revelation"', '"snake.gear_recovery"',
                      '"fox.intel_persists"'):
            self.assertIn(label, STORY)
        self.assertIn("NextChapter != Progress->Chapter + 1", STORY)
        self.assertIn("Progress->Chapter = PreviousChapter;", STORY)
        self.assertIn("TArray<FName> CompletedBeatIds;", STORY_HEADER)

    def test_player_controls_capture_instead_of_forced_teleport(self):
        self.assertIn('TEXT("3F_DetentionTransfer")', MISSION)
        self.assertIn("!bPOWRescued", MISSION)
        self.assertIn("Story->AdvanceChapter(1)", MISSION)
        self.assertIn('DeleteGameInSlot(TEXT("GrayFoxSliceV1"), 0)', MISSION)

    def test_fox_intel_and_return_are_transactions(self):
        self.assertIn('Story->RecordBeat(TEXT("fox.revelation")', PRISON)
        self.assertIn('Story->RecordBeat(TEXT("fox.intel_persists")', PRISON)
        self.assertIn("Story->AdvanceChapter(2)", PRISON)
        self.assertIn("ReturnToOuterHeavenSeconds", PRISON)
        self.assertIn("UGameplayStatics::OpenLevel(", PRISON)

    def test_equipment_recovery_is_real_and_saved(self):
        self.assertIn("GearLocker = SpawnBlock", PRISON)
        self.assertIn("RestoreCapturedGear(AMGPlayerCharacter* Player)", PRISON)
        self.assertIn("bGearRecovered && (Flags & MG_FOX_EXIT_WALL_BREACHED)", PRISON)
        self.assertIn("Save->bGearRecovered = bGearRecovered;", PRISON)
        self.assertIn("SchemaVersion = 2", PRISON_HEADER)
        self.assertIn('TEXT("MissionOneV1")', PRISON)

    def test_consumables_stay_spent_after_prison_exit(self):
        self.assertIn("PersistRecoveredInventory(AMGPlayerCharacter* Player)", PRISON)
        self.assertIn("OpeningSave->HandgunRounds =", PRISON)
        self.assertIn("OpeningSave->RationsRemaining =", PRISON)
        self.assertGreaterEqual(PLAYER.count("Prison->PersistRecoveredInventory(this)"), 2)

    def test_radio_and_intel_are_replayable(self):
        self.assertIn("Story->GetOpeningRadio(Channel)", HUD)
        self.assertIn("Story->GetIntelLog()", HUD)
        self.assertIn("MISSION INTEL / J TO CLOSE", HUD)

    def test_rust_and_unreal_guard_same_chapter_dependencies(self):
        self.assertIn("pub mod campaign;", RUST_LIB)
        for name in ("FIRST_PRISONER_INTEL", "GRAY_FOX_REPORT",
                     "GEAR_RECOVERED", "PRISON_EXIT_COMPLETE"):
            self.assertIn(name, RUST)
        self.assertIn("fn actual_escape_and_gear_are_required_to_finish", RUST)


if __name__ == "__main__":
    unittest.main()
