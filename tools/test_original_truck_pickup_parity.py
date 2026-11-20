"""Source-level parity checks for the MSX2 first-floor supply-truck rewards.

These checks verify authored code contracts, not UE5.7 compilation, spatial
playability, camera occlusion, final art quality or real-player enjoyment.
Browser behaviour is separately exercised by browser/tests/test_campaign.cjs.
"""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
CPP = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGGrayFoxMission.cpp").read_text(encoding="utf-8")
HEADER = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Public/MGGrayFoxMission.h").read_text(encoding="utf-8")
SAVE = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Public/MGGrayFoxSaveGame.h").read_text(encoding="utf-8")
EQUIP = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGEquipmentComponent.cpp").read_text(encoding="utf-8")
PLAYER = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGPlayerCharacter.cpp").read_text(encoding="utf-8")
BROWSER = (ROOT / "browser/game.js").read_text(encoding="utf-8")


class OriginalTruckPickupParity(unittest.TestCase):
    def test_all_three_truck_rewards_are_physical_and_awarded(self):
        for name, ident in (
            ("Ration", "RATION"),
            ("Card1", "CARD1"),
            ("Binoculars", "BINOCULARS"),
        ):
            self.assertIn("FirstFloor" + name + "Pickup=SpawnBlock", CPP)
            self.assertIn("CanReach(Player, FirstFloor" + name + "Pickup, 165.f)", CPP)
            self.assertIn('Inventory->AwardItem(TEXT("' + ident + '"), 1)', CPP)
            self.assertIn("bFirstFloor" + name + "Earned = true;", CPP)
        self.assertIn('FName(TEXT("BINOCULARS"))', EQUIP)
        self.assertIn('FName(TEXT("CARD1"))', EQUIP)
        self.assertIn('FName(TEXT("RATION")), TEXT("RATION"), 0, false, true', EQUIP)

    def test_finite_resources_and_legacy_save_migration(self):
        self.assertIn("Save->SchemaVersion = 7;", CPP)
        self.assertIn("uint32 SchemaVersion = 7;", SAVE)
        for version in range(2, 8):
            self.assertIn("Save->SchemaVersion == " + str(version), CPP)
        for field in ("bEarnedCard1", "bEarnedTruckRation", "bEarnedBinoculars"):
            self.assertIn("Save->" + field, CPP)
            self.assertIn(field, SAVE)
        self.assertIn("Save->TruckRationsRemaining = Player && Player->GetEquipment()", CPP)
        self.assertIn("LoadedTruckRationsRemaining = Save->SchemaVersion >= 7", CPP)
        self.assertIn('Inventory->AwardItem(TEXT("RATION"), LoadedTruckRationsRemaining)', CPP)
        self.assertIn("NotifyItemConsumed(AMGPlayerCharacter* Player)", CPP)
        self.assertIn("NotifyItemConsumed(AMGPlayerCharacter* Player)", HEADER)
        self.assertIn("Mission->NotifyItemConsumed(this)", PLAYER)
        self.assertIn("Save->bEarnedCard1 || Save->bEarnedTruckRation", CPP)
        self.assertIn("|| Save->bEarnedBinoculars", CPP)

    def test_scoping_zoom_never_creates_enemy_knowledge(self):
        self.assertIn('Equipment->IsItemEquipped(TEXT("BINOCULARS"))', PLAYER)
        self.assertIn("FollowCamera->SetFieldOfView(bBinocularView ? 34.f : 85.f)", PLAYER)
        self.assertIn("if (bBinocularView && (!Equipment || !Equipment->IsItemEquipped", PLAYER)
        self.assertIn("Math.PI/(world.scouting?5.6:player.aim?3.6:3.1)", BROWSER)
        self.assertIn("fov:Math.PI/(world.scouting?5.6:player.aim?3.6:3.1)", BROWSER)
        self.assertIn("world.scouting=false; // Optical view is input state", BROWSER)

    def test_browser_three_rewards_are_earned_and_not_respawned(self):
        for name in ("Ration", "Binoculars", "Card1"):
            self.assertIn("const original" + name + "Target=", BROWSER)
            self.assertIn("original" + name + "Prop.active=!", BROWSER)
        self.assertIn("rationRemaining--;syncEquipment();save()", BROWSER)
        self.assertIn("rationEarned,rationRemaining,binocularsEarned", BROWSER)
        self.assertIn("!lineBlockedSight(player,originalRationTarget,1.35,.4)", BROWSER)
        self.assertIn("!lineBlockedSight(player,originalBinocularsTarget,1.35,.4)", BROWSER)
        self.assertIn("!lineBlockedSight(player,originalCard1Target,1.35,.4)", BROWSER)


if __name__ == "__main__":
    unittest.main()
