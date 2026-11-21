"""Cross-engine source checks supplement the executable browser route regression."""
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
NATIVE = ROOT / 'unreal/MGRemaster/Source/MGRemaster/Private'
MISSION = (NATIVE / 'MGGrayFoxMission.cpp').read_text()
KID = (NATIVE / 'MGMachinegunKid.cpp').read_text()
BROWSER = (ROOT / 'browser/game.js').read_text()


class WestLiftParity(unittest.TestCase):
    def test_breach_requires_a_collected_charge_and_an_idle_fuse(self):
        self.assertIn('bServiceChargeCollected && BreachRemaining < 0.f', MISSION)
        self.assertIn('BreachRemaining = 2.4f;', MISSION)
        self.assertIn("service.charge?'PLACE CHARGE':'INSPECT MASONRY'", BROWSER)
        self.assertIn("action==='PLACE CHARGE'&&service.fuse<0", BROWSER)

    def test_boss_recovery_and_new_hits_use_current_sight(self):
        self.assertIn('Remaining = 1.8f;', KID)
        self.assertIn('bSeen && FMath::Abs(P.Y-MarkedY)<75.f', KID)
        self.assertIn("kid.phase='reload';kid.clock=1.8", BROWSER)
        self.assertIn('sees&&Math.abs(player.z-kid.lane)<.75', BROWSER)

    def test_parachute_cannot_be_picked_up_before_victory(self):
        self.assertIn('bKidDefeated && !bParachuteTaken && CanReach', MISSION)
        self.assertIn('service.kidDown&&!service.parachute&&dist', BROWSER)
        self.assertIn('bParachuteTaken = bKidDefeated && Save->bParachuteTaken;', MISSION)
        self.assertIn('service.parachute=service.kidDown&&data.service?.parachute===true;', BROWSER)
