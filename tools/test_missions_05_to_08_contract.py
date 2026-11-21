"""Source-contract guards for numbered missions 5 through 8.

Run: python -m unittest tools/test_missions_05_to_08_contract.py
These checks verify written C++ links and the original MSX2 finale order,
NOT Unreal compilation or playability. The J2ME v3.3.0 a1 room data remains
untraced; design geometry is explicitly provisional.
"""
from __future__ import annotations
from pathlib import Path
import re
import unittest

ROOT=Path(__file__).resolve().parents[1]
PUBLIC=ROOT/"unreal/MGRemaster/Source/MGRemaster/Public"
PRIVATE=ROOT/"unreal/MGRemaster/Source/MGRemaster/Private"

def get(path):
    return path.read_text(encoding="utf-8")
LATER=get(PRIVATE/"MGLaterMission.cpp")
FINALE=get(PRIVATE/"MGLaterMissionFinale.cpp")
HEADER=get(PUBLIC/"MGLaterMission.h")
CAMPAIGN=get(PRIVATE/"MGCampaignState.cpp")
BOSS=get(PRIVATE/"MGSetpieceBoss.cpp")
EQUIPMENT=get(PRIVATE/"MGEquipmentComponent.cpp")

class LaterMissionTests(unittest.TestCase):
    def test_every_numbered_world_constructed(self):
        for n,fn in {3:"BuildExterior",4:"BuildBuildingTwo",
                     5:"BuildEllenReturn",6:"BuildDesertAndCowardDuck",
                     7:"BuildDeepBunker",8:"BuildTX55Finale"}.items():
            self.assertIn(f"if (Mission=={n}) {fn}()",LATER)
            self.assertIn(f"void AMGLaterMission::{fn}()",LATER+FINALE)

    def test_interaction_route_is_reachable_from_actor(self):
        self.assertIn("return TryFinaleInteraction(Player);",LATER)
        self.assertIn("HandleMissionHazards(Dt);",LATER)
        for name in ["HandleMissionHazards","TryFinaleInteraction"]:
            self.assertIn(f"AMGLaterMission::{name}(",FINALE)

    def test_madnar_to_desert_not_an_empty_level(self):
        self.assertIn("M05_ResearchToDesertPassage",LATER)
        self.assertIn("AdvanceAndReload(6)",FINALE)
        self.assertIn("madnar.weakness",FINALE)

    def test_three_real_coward_duck_hostages(self):
        for required in ["JenniferBrother=Block","HostageLeft=Block",
                         "HostageRight=Block","m06.coward_duck_defeated",
                         "brother.escape_ladder","m06.hostage_left_rescued",
                         "m06.hostage_right_rescued"]:
            self.assertIn(required,FINALE)
        self.assertIn("NotifyHostageShield",BOSS)
        self.assertIn("M06_JenniferBrother_Hostage",BOSS)
        self.assertIn("m06.card8_acquired",CAMPAIGN)

    def test_compass_and_oxygen_on_real_map(self):
        self.assertIn('TEXT("OXYGEN_TANK")',EQUIPMENT)
        self.assertIn('TEXT("COMPASS")',FINALE)
        self.assertIn("WaterDamageAccumulator",FINALE)
        self.assertIn('TEXT("M06_WaterPassageFloor")',FINALE)

    def test_two_bunker_security_choices_and_source_witness(self):
        for name in ["M07_SecurityOverrideTerminal","M07_MaintenanceBypassWall",
                     "pow.final_reveal","boss.abort_order",
                     "snake.disobeys_abort","m07.final_security_cleared"]:
            self.assertIn(name,FINALE)
        self.assertIn("AdvanceAndReload(8)",FINALE)

    def test_txs_fifteen_known_and_sixteenth_right(self):
        match=re.search(r"RightFootSequence\[\]\s*=\s*\{([^}]+)\}",CAMPAIGN)
        self.assertIsNotNone(match)
        seq=[x.strip().lower()=="true" for x in match.group(1).split(",")]
        self.assertEqual(len(seq),16)
        self.assertEqual(seq,[True,True,False,True,False,False,True,False,
                              False,True,True,False,True,False,True,True])
        self.assertIn("State->RecordTX55Charge(bRight)",FINALE)
        self.assertIn('TEXT("tx55.destruction")',FINALE)

    def test_txs_destruction_before_commander_and_escape(self):
        i=FINALE.index('State->RecordBeat(TEXT("tx55.destruction")')
        j=FINALE.index('SpawnBoss(TEXT("m08.big_boss_defeated")',i)
        self.assertLess(i,j)
        self.assertIn('TEXT("ROCKET_LAUNCHER")',FINALE)
        self.assertIn("LeftEscapeLadder",FINALE)
        self.assertIn("RightEscapeLadder",FINALE)
        self.assertIn("brother.escape_ladder",FINALE)
        self.assertIn("EscapeSaveAccumulator",FINALE)

    def test_end_to_end_checkpoint_ids(self):
        for identifier in ["m04.card7_claimed","m06.card8_acquired",
                           "snake.escaped_outer_heaven"]:
            self.assertIn(identifier,LATER+FINALE+CAMPAIGN)
        self.assertIn("Save->ExtraOwnedItems.AddUnique",LATER)
        self.assertIn("Save->ExtraOwnedWeapons.AddUnique",LATER)

    def test_source_production_gates_are_not_misrepresented(self):
        self.assertIn("NOT verified",FINALE)
        self.assertIn("provisional",FINALE)
        self.assertNotIn("gameplay_verified",FINALE)
        self.assertNotIn("J2ME_verified",FINALE)

if __name__=="__main__":
    unittest.main()
