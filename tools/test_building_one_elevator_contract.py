"""Static floor and lift-contract checks, not a substitute for UE compilation."""
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = json.loads((ROOT / "data/building_one_vertical_contract.json").read_text(encoding="utf-8"))
CPP = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGMissionOne.cpp").read_text(encoding="utf-8")
HEADER = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Public/MGMissionOne.h").read_text(encoding="utf-8")
GUARD = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGGuardActor.cpp").read_text(encoding="utf-8")
HUD = (ROOT / "unreal/MGRemaster/Source/MGRemaster/Private/MGPrototypeHUD.cpp").read_text(encoding="utf-8")


class BuildingOneElevatorContractTests(unittest.TestCase):
    def test_all_three_floors_have_unique_deck_heights(self):
        floors = CONTRACT["decks"]
        self.assertEqual([f["floor"] for f in floors], [1, 2, 3])
        self.assertEqual([f["z_m"] for f in floors], [0, 6, 12])
        self.assertIn('for (int32 Floor = 2; Floor <= 3; ++Floor)', CPP)
        self.assertIn('FloorLiftLocation(int32 Floor)', CPP)

    def test_lift_travel_is_stateful_bidirectional_and_restartable(self):
        for signature in ("CycleLiftDestination(", "StartLift(", "FinishLift(", "bLiftTravelling",
                          "SetMovementMode(MOVE_Walking)", "SaveProgress(Player)", "CurrentFloor = DestinationFloor"):
            self.assertIn(signature, CPP)
        self.assertIn("ResumeLocation() const", HEADER)
        self.assertIn("Lift->IsLiftTravelling()", HUD)

    def test_guard_hearing_and_pickups_are_deck_local(self):
        self.assertIn("Abs(Player->GetActorLocation().Z - GetActorLocation().Z) > 260.f", GUARD)
        self.assertIn("FVector::Dist(Player->GetActorLocation(), P.Location)", CPP)

    def test_handgun_ammo_is_independent_of_weapon_ownership(self):
        self.assertIn('AddPickup(TEXT("P07"), TEXT("HANDGUN_AMMO")', CPP)
        self.assertIn('Equipment->AwardAmmunition(TEXT("HANDGUN"), 12)', CPP)
        self.assertEqual(CONTRACT["upper_pickups"][0]["floor"], 3)

    def test_floor_two_and_three_guards_share_runtime_ids(self):
        for guard in CONTRACT["upper_guard_patrols"]:
            self.assertIn(f'TEXT("{guard["id"]}_', CPP)
            self.assertGreaterEqual(len(guard["waypoints_xy_m"]), 2)

    def test_save_schema_and_rescue_event(self):
        self.assertIn("Save->Version = 2;", CPP)
        self.assertIn("Save->bPOWRescued = bPOWRescued;", CPP)
        self.assertIn("PRISONER CONTACT", CPP)


if __name__ == "__main__":
    unittest.main()
