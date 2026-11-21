"""Source contracts for shared browser/Unreal playability pass (2026-11-21).

Does NOT compile Unreal, draw the browser, or simulate a human playtest.
Those require native platform runs and real-player observation.
"""
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
def read(path):
    return (ROOT / path).read_text(encoding="utf-8")


class CrossEnginePlayabilitySourceTests(unittest.TestCase):
    def test_xbox_face_buttons_match_and_cinematic_skip_really_binds(self):
        ini = read("unreal/MGRemaster/Config/DefaultInput.ini")
        pairs = (
            ("Interact", "Gamepad_FaceButton_Bottom"),  # A
            ("Melee", "Gamepad_FaceButton_Left"),       # X
            ("Crouch", "Gamepad_FaceButton_Right"),     # B
            ("Prone", "Gamepad_FaceButton_Top"),        # Y
            ("UseItem", "Gamepad_DPad_Down"),
            ("SkipCinematic", "Gamepad_FaceButton_Right"),
            ("SkipCinematic", "Escape"),
        )
        for action, key in pairs:
            line = f'+ActionMappings=(ActionName="{action}",Key={key})'
            self.assertIn(line, ini)
        self.assertNotIn(
            '+ActionMappings=(ActionName="Interact",Key=Gamepad_FaceButton_Left)', ini
        )
        self.assertNotIn(
            '+ActionMappings=(ActionName="Melee",Key=Gamepad_FaceButton_Bottom)', ini
        )
        browser = read("browser/game.js")
        for js in ("if(down(0))action('interact')",
                   "if(down(2))action('melee')",
                   "if(down(3))action('prone')",
                   "if(down(13))action('useitem')"):
            self.assertIn(js, browser)
        hud = read("unreal/MGRemaster/Source/MGRemaster/Private/MGPrototypeHUD.cpp")
        self.assertIn("E / A INTERACT", hud)
        self.assertIn("F / X MELEE", hud)
        self.assertIn("Z / Y PRONE", hud)
        self.assertIn("ESC / B SKIP", hud)
        player = read("unreal/MGRemaster/Source/MGRemaster/Private/MGPlayerCharacter.cpp")
        self.assertIn("Mission->IsStoryCinematicPlaying()) return;", player)

    def test_player_retries_never_delete_the_unreal_save_slot(self):
        mission = read("unreal/MGRemaster/Source/MGRemaster/Private/MGGrayFoxMission.cpp")
        player = read("unreal/MGRemaster/Source/MGRemaster/Private/MGPlayerCharacter.cpp")
        declaration = read("unreal/MGRemaster/Source/MGRemaster/Public/MGGrayFoxMission.h")
        self.assertIn("void RetryFromCheckpoint();", declaration)
        self.assertIn("void AMGGrayFoxMission::RetryFromCheckpoint()", mission)
        section = mission.split("void AMGGrayFoxMission::RetryFromCheckpoint()",1)[1]
        self.assertIn("GetCurrentLevelName(this, true)", section)
        self.assertNotIn("DeleteGameInSlot", section)
        self.assertNotIn("DeleteGameInSlot", mission)
        self.assertGreaterEqual(player.count("Mission->RetryFromCheckpoint();"),2)

    def test_browser_retry_and_unreadable_continue_remain_non_destructive(self):
        browser = read("browser/game.js")
        html = read("browser/index.html")
        self.assertIn("function retryCheckpoint()", browser)
        self.assertIn("if(health<=0){retryCheckpoint();return}", browser)
        self.assertIn("if(location.hash==='#retry-checkpoint')return true", browser)
        self.assertIn("if(resume&&!load())", browser)
        self.assertIn("CHECKPOINT UNREADABLE", browser)
        self.assertNotIn("localStorage.removeItem(STORAGE)", browser)
        self.assertIn("RETRY FROM CHECKPOINT", html)
        self.assertIn("requestNewOperation", browser)
        self.assertIn("confirmTitleChoice===1", browser)

    def test_item_selection_never_consumes_rations_or_emits_smoke(self):
        browser = read("browser/game.js")
        equip = browser.split("else if(name==='equipclose'){",1)[1].split(
            "else if(name==='useitem'){",1
        )[0]
        use = browser.split("else if(name==='useitem'){",1)[1].split(
            "else if(name==='next'||name==='previous')",1
        )[0]
        self.assertIn("equippedItemSlot=slot", equip)
        self.assertNotIn("rationRemaining--", equip)
        self.assertNotIn("world.smokeUntil=gameTime+3", equip)
        for term in ("rationRemaining--", "world.scouting=!world.scouting",
                     "world.smokeUntil=gameTime+3", "save()"):
            self.assertIn(term, use)
        self.assertIn("equippedItemName:equippedItemSlot.name", browser)

    def test_hud_context_is_real_local_reach_not_omniscient_marker(self):
        mission = read("unreal/MGRemaster/Source/MGRemaster/Private/MGGrayFoxMission.cpp")
        declaration = read("unreal/MGRemaster/Source/MGRemaster/Public/MGGrayFoxMission.h")
        hud = read("unreal/MGRemaster/Source/MGRemaster/Private/MGPrototypeHUD.cpp")
        self.assertIn("GetContextActionPrompt", declaration)
        context = mission.split(
            "FString AMGGrayFoxMission::GetContextActionPrompt(",1
        )[1].split("FString AMGGrayFoxMission::GetPacingHint()",1)[0]
        self.assertIn("return CanReach(Player, Object, Cm)", context)
        for token in ("FirstFloorRationPickup", "FirstFloorCard1Pickup",
                      "POWInformant", "WeakWall", "FoxProxy", "GearDoor",
                      "Card2Door", "ChapterExit", "RoofLauncher", "TankMineMarkers"):
            self.assertIn(token, context)
        self.assertNotIn("SetActorLocation", context)
        self.assertNotIn("SaveProgress", context)
        self.assertNotIn("ApplyAction", context)
        self.assertIn("Mission->GetContextActionPrompt(Player)", hud)
        self.assertIn("optionalStageHelp(tier)", read("browser/game.js"))


if __name__ == "__main__":
    unittest.main()
