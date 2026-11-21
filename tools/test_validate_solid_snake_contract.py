"""Self-contained regression examples for the protagonist contract validator."""
import unittest
from validate_solid_snake_contract import validate


class TestSnakeContract(unittest.TestCase):
    def setUp(self):
        self.data = {
            "schema_version": 1,
            "status": "source_contract_only",
            "doctrine": "DOCTRINE_000",
            "identity": {
                "character": "Solid Snake", "era": 1995,
                "variant_ids": ["snake_mg1_1995_infiltration", "snake_mg1_1995_disarmed"]},
            "animation_architecture": {
                "weapon_socket": "weapon_r",
                "required_layers": ["base_locomotion","stance","upper_body_aim_offset",
                                    "two_hand_ik","foot_ik","impact_additive","facial_look_at"]},
            "animation_sets": {
                "idle": ["a"+str(i) for i in range(6)],
                "locomotion": ["b"+str(i) for i in range(13)],
                "crouch": ["c"+str(i) for i in range(5)],
                "prone": ["d"+str(i) for i in range(6)],
                "cover": ["e"+str(i) for i in range(12)],
                "weapon": ["f"+str(i) for i in range(9)],
                "melee": ["g"+str(i) for i in range(7)],
                "narrative": ["h"+str(i) for i in range(6)],
                "face": ["i"+str(i) for i in range(5)]},
            "required_test_cases": ["captured_unarmed_aim","aim_with_weapon_both_shoulders",
                                    "wall_knock_not_player_location","breach_action_priority",
                                    "save_reload_variant","first_time_identity_720p"],
            "quality_gates": [{"id": x, "status": "unverified", "evidence": []}
                for x in ["identity","rig","locomotion","aim","interaction","narrative",
                          "audio_accessibility","performance","browser_parity","player_study"]],
            "source_status": {"ue_compiled": False},
            "required_control_actions": ["Aim", "MoveForward"]}

    def validate(self):
        return validate(self.data, "Aiming prone gray fox UNVERIFIED weapon_r Doctrine 000 1995",
                        'ActionName="Aim" AxisName="MoveForward"')

    def test_valid_contract(self):
        self.assertEqual(self.validate(), [])

    def test_detects_false_completion(self):
        self.data["status"] = "complete"
        self.assertIn("specification may not claim a delivered character", self.validate())

    def test_detects_missing_aim(self):
        self.data["animation_architecture"]["required_layers"].remove("upper_body_aim_offset")
        self.assertTrue(any("upper_body_aim_offset" in msg for msg in self.validate()))

    def test_detects_unsupported_variant(self):
        self.data["identity"]["variant_ids"] = ["mgs4_old_snake"]
        self.assertIn("infiltration/disarmed variants missing or renamed", self.validate())

    def test_detects_false_verification(self):
        self.data["quality_gates"][0]["status"] = "verified"
        self.assertTrue(any("without evidence" in msg for msg in self.validate()))


if __name__ == "__main__":
    unittest.main()
