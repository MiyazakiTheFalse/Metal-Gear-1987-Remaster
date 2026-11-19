"""Static and binary smoke checks for the *source* Shotmaker encounter.

Run locally without paid CI: python -m unittest discover -s tools -p 'test_*.py'
This does not compile Unreal or prove a working rig/cutscene.
"""
import ast
import json
from pathlib import Path
import unittest
import wave

ROOT = Path(__file__).resolve().parents[1]
BASE = ROOT / "unreal/MGRemaster/Source/MGRemaster"
MISSION = (BASE / "Private/MGGrayFoxMission.cpp").read_text(encoding="utf-8")
BOSS = (BASE / "Private/MGShootGunnerActor.cpp").read_text(encoding="utf-8")
HEADER = (BASE / "Public/MGShootGunnerActor.h").read_text(encoding="utf-8")
PLAYER = (BASE / "Private/MGPlayerCharacter.cpp").read_text(encoding="utf-8")
EQUIPMENT = (BASE / "Private/MGEquipmentComponent.cpp").read_text(encoding="utf-8")
HUD = (BASE / "Private/MGPrototypeHUD.cpp").read_text(encoding="utf-8")
CONTRACT = json.loads((ROOT / "data/shotmaker_attack_contract.json").read_text())
ASSETS = ROOT / "assets/source/shotmaker"

class ShotmakerSourceChecks(unittest.TestCase):
    def test_canon_and_editable_tuning_not_conflated(self):
        facts = CONTRACT["canon_facts"]
        self.assertEqual(facts["burst_shots"], 3)
        self.assertTrue(facts["wide_projectiles"])
        self.assertTrue(facts["player_begins_disarmed"])
        for name in ("WindupSeconds", "IntervalSeconds", "RecoverySeconds",
                     "HalfSpreadDegrees", "PelletCount", "MaxShotDamage"):
            self.assertIn(name, HEADER)
        self.assertIn("ShotsFired >= 3", BOSS)
        self.assertIn("EAttackState::Warning", BOSS)
        self.assertIn("EAttackState::Recovery", BOSS)

    def test_cover_and_muzzle_are_physical(self):
        for label in ("BossLowCoverA", "BossLowCoverB", "BossLowCoverC"):
            line = next(l for l in MISSION.splitlines() if f'TEXT("{label}")' in l)
            self.assertIn('-25.f', line)
            self.assertIn('1.5f', line)
        self.assertIn("LineTraceSingleByChannel(Hit, Muzzle", BOSS)
        self.assertIn("Hit.GetActor() == Player", BOSS)
        self.assertIn("ShotmakerSight", BOSS)
        self.assertIn("ECC_Visibility", BOSS)

    def test_route_and_flag_awards_are_earned(self):
        for needed in ("CreateBossWing()", "TryPunchRecoveryDoor",
                       "MG_FOX_ENTER_SHOTMAKER", "MG_FOX_RECOVER_GEAR",
                       "MG_FOX_DEFEAT_SHOTMAKER", "MG_FOX_COLLECT_CARD3"):
            self.assertIn(needed, MISSION)
        self.assertIn("CARD2", MISSION)
        self.assertIn("MG_FOX_CARD3_RECOVERED", MISSION)
        self.assertIn("IsHandgunEquipped", PLAYER)
        self.assertIn("ConsumeHandgunRound", PLAYER)
        self.assertIn("Shotmaker->ReceiveTrackerPing", MISSION)
        self.assertIn("DiscardPlantedTransmitter", EQUIPMENT)
        self.assertIn("IsTransmitterEquipped", PLAYER)

    def test_radar_and_warning_match_boss_wing(self):
        self.assertIn("bBossWing", HUD)
        self.assertIn("TActorIterator<AMGShootGunnerActor>", HUD)
        self.assertIn("SHOTGUN AIMING", HUD)
        self.assertIn("IsTransmitterActive()", HUD)

    def test_one_editable_obj_and_one_game_import_obj(self):
        edit = (ASSETS / "riot_gun_proxy.obj").read_text(encoding="utf-8")
        game = (ASSETS / "riot_gun_game_mesh.obj").read_text(encoding="utf-8")
        self.assertGreaterEqual(edit.count("\no "), 15)
        self.assertEqual(game.count("\no "), 1)
        self.assertEqual(game.count("\ng "), edit.count("\no "))
        self.assertEqual(game.count("\nv "), edit.count("\nv "))
        self.assertIn("mtllib riot_gun_proxy.mtl", game)
        self.assertGreaterEqual(game.count("\nf "), 100)

    def test_original_wav_files_have_real_header_and_audio(self):
        for name in ("Shotmaker_AimCue", "Shotmaker_Blast", "Shotmaker_Cycle"):
            path = ASSETS / "audio" / f"{name}.wav"
            with wave.open(str(path), "rb") as w:
                self.assertEqual(w.getframerate(), 22050)
                self.assertEqual(w.getnchannels(), 1)
                self.assertEqual(w.getsampwidth(), 2)
                self.assertGreater(w.getnframes(), 3000)
            self.assertIn(name, BOSS)

    def test_editor_script_parses_and_refs_exact_paths(self):
        script = (ROOT / "unreal/MGRemaster/Content/Python/import_shotmaker_source_assets.py").read_text()
        ast.parse(script)
        for fragment in ("SM_RiotShotgun_1995", "Shotmaker_Blast",
                         "replace_existing", "does_asset_exist"):
            self.assertIn(fragment, script)

if __name__ == "__main__":
    unittest.main()
