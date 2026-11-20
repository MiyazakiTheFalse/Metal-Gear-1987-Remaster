"""Static contract tests for authentic whole-game conversion and camera counterplay.

Run with: python -m unittest tools.test_full_campaign_inventory_and_cctv_source
No Unreal compilation, editor import, AI/nav runtime or playtesting is asserted.
"""
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "unreal/MGRemaster/Source/MGRemaster"
PLAN = json.loads((ROOT / "data/MSX2_FULL_CAMPAIGN_CONVERSION_PLAN_1995.json").read_text())
EQUIP = (SOURCE / "Private/MGEquipmentComponent.cpp").read_text()
EQUIP_H = (SOURCE / "Public/MGEquipmentComponent.h").read_text()
SAVE = (SOURCE / "Public/MGGrayFoxSaveGame.h").read_text()
MISSION = (SOURCE / "Private/MGGrayFoxMission.cpp").read_text()
MISSION_H = (SOURCE / "Public/MGGrayFoxMission.h").read_text()
PLAYER = (SOURCE / "Private/MGPlayerCharacter.cpp").read_text()
CAMERA = (SOURCE / "Private/MGSurveillanceCameraActor.cpp").read_text()
HUD = (SOURCE / "Private/MGPrototypeHUD.cpp").read_text()


class WholeGameSourceAudit(unittest.TestCase):
    def test_full_msx2_boss_and_mission_contract(self):
        self.assertEqual(len(PLAN["phases"]), 10)
        self.assertEqual(len(PLAN["bosses"]), 11)
        self.assertEqual(PLAN["authenticWeapons"], [
            "handgun", "mine", "plastic_explosive", "remote_missile",
            "grenade_launcher", "smg", "rocket_launcher"
        ])
        self.assertEqual(PLAN["nonCanonTools"], ["stun"])
        self.assertEqual(PLAN["currentlyImplementedBossSetPieces"], 1)
        self.assertEqual(
            [x["id"] for x in PLAN["bosses"]],
            ["four_mercenaries", "shoot_gunner", "machinegun_kid", "hind_d",
             "tank", "bulldozer", "fire_trooper", "tx11_pair", "coward_duck",
             "tx55", "big_boss"]
        )
        self.assertTrue(all(not x["unrealVerified"] for x in PLAN["phases"]))
        self.assertIn("NES Twin Shot", PLAN["versionExclusions"][0])
        self.assertIn("NES Super Computer", PLAN["versionExclusions"][1])

    def test_only_authentic_weapons_in_normal_inventory(self):
        # The noncanonical gun must only be added explicitly within the
        # editor/nonshipping opt-in block, never initialised by default.
        self.assertIn("bEnableNonCanonicalTestWeapons = false", EQUIP_H)
        self.assertIn("#if !UE_BUILD_SHIPPING", EQUIP)
        self.assertIn("if (bEnableNonCanonicalTestWeapons)", EQUIP)
        self.assertEqual(EQUIP.count('Weapons.Add({FName(TEXT("STUN"))'), 1)
        initial = EQUIP.split('    Weapons = {', 1)[1].split('    };', 1)[0]
        self.assertNotIn('FName(TEXT("STUN"))', initial)
        for w in ("HANDGUN", "MINE", "PLASTIC_EXPLOSIVE",
                  "REMOTE_MISSILE", "GRENADE_LAUNCHER",
                  "SMG", "ROCKET_LAUNCHER"):
            self.assertIn(f'FName(TEXT("{w}"))', initial)
        self.assertIn('Id != FName(TEXT("HANDGUN"))', EQUIP)
        self.assertNotIn("24, true, false", initial)  # No starting sidearm.

    def test_real_early_handgun_pickup_and_checkpoint_schema(self):
        self.assertIn("Building1EarlyHandgunCache", MISSION)
        self.assertIn("CanReach(Player, EarlyHandgunPickup, 185.f)", MISSION)
        self.assertIn('Inventory->AwardWeapon(TEXT("HANDGUN"), 24)', MISSION)
        self.assertIn("bEarlyHandgunAcquired = true", MISSION)
        self.assertIn("SaveProgress(Player); // Independent V4 earned-gun state.", MISSION)
        self.assertIn("Save->SchemaVersion = 4", MISSION)
        self.assertIn("Save->bEarnedHandgun = bEarlyHandgunAcquired", MISSION)
        self.assertIn("Save->SchemaVersion < 4 || Save->bEarnedHandgun", MISSION)
        self.assertIn("bEarlyHandgunAcquired && IsValid(EarlyHandgunPickup)", MISSION)
        self.assertIn("uint32 SchemaVersion = 4", SAVE)
        self.assertIn("bool bEarnedHandgun = false", SAVE)
        self.assertIn("int32 HandgunRounds = 0", SAVE)
        self.assertIn("bLoadedHandgunAcquired", MISSION_H)
        self.assertIn("HasInitializedInventory()", EQUIP_H)
        self.assertIn("bInventorySlotsInitialized = true;", EQUIP)
        self.assertIn("!Player->GetEquipment()->HasInitializedInventory()", MISSION)

    def test_cctv_counterplay_requires_physical_evidence(self):
        self.assertIn("MGCCTVPhysicalLOS", CAMERA)
        self.assertIn("Hit.GetActor() == Player", CAMERA)
        self.assertIn("VisualContactSeconds >= 1.25f", CAMERA)
        self.assertIn("OnVisualContactAcquired", CAMERA)
        self.assertIn("OnVisualContactLost", CAMERA)
        self.assertIn("NotifyGuardContactReported(LastObservedWorldPosition)", CAMERA)
        self.assertIn("ReceiveLocalSquadReport(LastObservedWorldPosition, this)", CAMERA)
        self.assertIn("MGCCTVPanelReach", CAMERA)
        self.assertIn("Disable(true)", CAMERA)
        self.assertIn("Disable(false)", CAMERA)
        self.assertIn("TryDisableAtControlPanel(this)", PLAYER)
        self.assertIn("bStun && Cast<AMGSurveillanceCameraActor>", PLAYER)
        self.assertIn("STUN DART HAS NO EFFECT ON CCTV", PLAYER)
        self.assertIn("CCTV ACQUIRING", HUD)
        self.assertIn("bPostCard3", HUD)
        self.assertEqual(MISSION.count("CCTV->ConfigureSector("), 2)

    def test_ui_and_design_status_never_claim_final_import(self):
        for b in PLAN["bosses"]:
            self.assertNotEqual(b["implementationStatus"], "ue_verified")
            self.assertFalse(b["unrealVerified"])
        self.assertIn("not UE5.7 compiled or playtested",
                      (ROOT/"docs/FULL_GAME_CONVERSION_AND_BOSS_STATUS_2026_11_20.md").read_text())

if __name__ == "__main__":
    unittest.main()
