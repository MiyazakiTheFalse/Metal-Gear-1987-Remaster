"""Unreal/Browser Building 2 source contracts; never mistaken for a UE PIE test."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
BROWSER = (ROOT/"browser/game.js").read_text(encoding="utf-8")
MISSION = (ROOT/"unreal/MGRemaster/Source/MGRemaster/Private/MGGrayFoxMission.cpp").read_text(encoding="utf-8")
HEADER = (ROOT/"unreal/MGRemaster/Source/MGRemaster/Public/MGGrayFoxMission.h").read_text(encoding="utf-8")
SAVE = (ROOT/"unreal/MGRemaster/Source/MGRemaster/Public/MGGrayFoxSaveGame.h").read_text(encoding="utf-8")
IMPORTER = (ROOT/"unreal/MGRemaster/Content/Python/import_field_character_glbs.py").read_text(encoding="utf-8")


class Building2SourceContracts(unittest.TestCase):
    def test_tank_is_mine_only_not_handgun_damage(self):
        self.assertIn("if(cmd==='PLANT MINE'", BROWSER)
        self.assertIn("b2.armed.splice", BROWSER)
        self.assertIn("ArmedTankMineSites.RemoveAt", MISSION)
        self.assertIn("--TankHitsRemaining", MISSION)
        self.assertIn("BroadcastStructuralImpact", MISSION)
        self.assertIn("TankMineStation", MISSION)

    def test_cover_and_warning_in_both(self):
        self.assertIn("tank shelter", BROWSER)
        self.assertIn("TankBlastRefuge", MISSION)
        self.assertIn("b2.phase='warning';b2.clock=1.1", BROWSER)
        self.assertIn("TankAttackPhase=1;TankPhaseRemaining=1.1f", MISSION)
        self.assertIn("lineBlockedSight(b2Tank,player", BROWSER)
        self.assertIn("SCENE_QUERY_STAT(TankTurretTrace)", MISSION)

    def test_interconnected_and_checkpointed(self):
        for token in ["DESCEND TO BUILDING 2", "RETURN TO ROOF", "syncB2();roofSync();"]:
            self.assertIn(token, BROWSER)
        self.assertIn("CanReach(Player,Building2Entry", MISSION)
        self.assertIn("CanReach(Player,Building2Return", MISSION)
        self.assertIn("bBuilding2Checkpoint", MISSION)
        self.assertIn("Save->SchemaVersion = 6;", MISSION)
        self.assertIn("uint32 SchemaVersion = 6;", SAVE)
        self.assertIn("version:10,b2:", BROWSER)
        self.assertIn("if(data.version>=10&&stage===10", BROWSER)
        self.assertIn("ArmedTankMineSites = ArmedTankMineSites", MISSION)
        self.assertIn("bTankDefeated = false", HEADER)

    def test_unreal_character_import_resolves_runtime_paths(self):
        for name in ["SK_Snake_1995_Infiltration", "SK_GrayFox_1995_Prisoner",
                     "SK_Guard_1995", "SK_Shotmaker_1995"]:
            self.assertIn(name, IMPORTER)
        self.assertIn('"/Game/Characters/Bosses/Shotmaker"', IMPORTER)
        self.assertIn('unreal.SkeletalMesh', IMPORTER)
        self.assertIn('does_asset_exist(target)', IMPORTER)


if __name__ == "__main__":
    unittest.main()
