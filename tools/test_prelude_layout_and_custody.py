"""Static geometry/guard-source audit: not an engine collision or navigation test."""
import re
import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
M=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Private/MGGrayFoxMission.cpp").read_text()
G=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Private/MGGuardActor.cpp").read_text()
H=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Public/MGGuardActor.h").read_text()

def block(name):
    pat=r'SpawnBlock\(TEXT\("'+re.escape(name)+r'"\), FVector\(([-.0-9f, ]+)\),\s*FVector\(([-.0-9f, ]+)\)'
    hit=re.search(pat,M)
    if not hit: raise ValueError("Cannot find source block "+name)
    def vec(x): return [float(y.strip().removesuffix("f")) for y in x.split(",")]
    return vec(hit.group(1)),vec(hit.group(2))

def inside(p,center,size):
    return abs(p[0]-center[0])<=size[0]*50 and abs(p[1]-center[1])<=size[1]*50

class PreludeLayoutDoctrineTests(unittest.TestCase):
    def test_visible_cover_reaches_guard_eye_height(self):
        for tag in ("PreludeUtility","PreludeCargoA","PreludeCargoB","GuardCover"):
            center,size=block(tag)
            self.assertGreater(center[2]+50*size[2],44.0,tag)
    def test_custody_officers_not_in_booth(self):
        c,s=block("PreludeSecurityBooth")
        guards=[(-1570,315,-11),(-1470,170,-11)]
        for guard in guards:self.assertFalse(inside(guard,c,s))
        self.assertIn("FVector(-1570.f, 315.f",M)
        self.assertIn("FVector(-1470.f, 170.f",M)
    def test_usable_approach_lane_between_truck_and_booth(self):
        c1,s1=block("CustodyTruckChassis")
        c2,s2=block("PreludeSecurityBooth")
        clearance=(c2[1]-s2[1]*50)-(c1[1]+s1[1]*50)
        self.assertGreaterEqual(clearance,170.0)
        self.assertLessEqual(clearance,380.0)
        self.assertIn("FVector(-1590.f, -15.f",M)
    def test_challenge_requires_evidence_and_reset(self):
        self.assertIn("ChallengeAge < .75f",G)
        self.assertIn("bHasLineOfSight",G)
        self.assertIn("CustodyThreatMemorySeconds",G)
        self.assertIn("OnCustodyChallenge()",G)
        self.assertIn("IsIssuingChallenge()",M)
    def test_debug_sightlines_are_opt_in(self):
        self.assertIn("bool bShowPerceptionDebug = false",H)
        self.assertIn("if (bShowPerceptionDebug)",G)

if __name__=="__main__": unittest.main()
