"""Static consistency checks for the binding 1995 Solid Snake bible.

This validates a *specification*, not an Unreal build or delivered animation assets.
"""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "data" / "solid_snake_character_contract.json"
BIBLE = ROOT / "docs" / "SOLID_SNAKE_1995_PROTAGONIST_BIBLE.md"
INPUT = ROOT / "unreal" / "MGRemaster" / "Config" / "DefaultInput.ini"

EXPECTED_SETS = {"idle", "locomotion", "crouch", "prone", "cover",
                 "weapon", "melee", "narrative", "face"}
REQUIRED_ASSETS = ("snake_mg1_1995_infiltration", "snake_mg1_1995_disarmed")
REQUIRED_GATES = {"identity", "rig", "locomotion", "aim", "interaction", "narrative",
                  "audio_accessibility", "performance", "browser_parity", "player_study"}


def validate(contract, bible, input_text):
    errors = []
    if contract.get("schema_version") != 1:
        errors.append("unsupported contract version")
    if contract.get("status") != "source_contract_only":
        errors.append("specification may not claim a delivered character")
    if contract.get("doctrine") != "DOCTRINE_000":
        errors.append("supreme doctrine missing")
    identity = contract.get("identity", {})
    if identity.get("era") != 1995 or identity.get("character") != "Solid Snake":
        errors.append("wrong protagonist or timeline")
    if set(identity.get("variant_ids", [])) != set(REQUIRED_ASSETS):
        errors.append("infiltration/disarmed variants missing or renamed")
    arch = contract.get("animation_architecture", {})
    if arch.get("weapon_socket") != "weapon_r":
        errors.append("weapon socket name does not match code")
    for key in ("base_locomotion", "stance", "upper_body_aim_offset",
                "two_hand_ik", "foot_ik", "impact_additive", "facial_look_at"):
        if key not in arch.get("required_layers", []):
            errors.append("animation layer missing: " + key)
    sets = contract.get("animation_sets", {})
    if set(sets) != EXPECTED_SETS:
        errors.append("an animation category is missing")
    animation_names = [x for values in sets.values() for x in values]
    if len(animation_names) != len(set(animation_names)):
        errors.append("duplicate animation ID")
    if len(animation_names) < 50:
        errors.append("insufficient animation coverage")
    essential_cases = {"captured_unarmed_aim", "aim_with_weapon_both_shoulders",
                       "wall_knock_not_player_location", "breach_action_priority",
                       "save_reload_variant", "first_time_identity_720p"}
    if not essential_cases.issubset(set(contract.get("required_test_cases", []))):
        errors.append("critical adversarial tests missing")
    gate_ids = {g.get("id") for g in contract.get("quality_gates", [])}
    if gate_ids != REQUIRED_GATES:
        errors.append("quality gates missing")
    for gate in contract.get("quality_gates", []):
        if gate.get("status") == "verified" and not gate.get("evidence"):
            errors.append("gate marked verified without evidence: " + gate.get("id", "?"))
    for key, value in contract.get("source_status", {}).items():
        if not isinstance(value, bool):
            errors.append("non-bool source_status: " + key)
    bindings = contract.get("required_control_actions", [])
    for binding in bindings:
        if 'ActionName="' + binding + '"' not in input_text and 'AxisName="' + binding + '"' not in input_text:
            errors.append("unbound control action: " + binding)
    for anchor in ("Aiming", "Prone", "Gray Fox", "UNVERIFIED",
                   "weapon_r", "Doctrine 000", "1995"):
        if anchor.lower() not in bible.lower():
            errors.append("bible missing anchor: " + anchor)
    return errors


def main():
    contract = json.loads(CONTRACT.read_text(encoding="utf-8"))
    errors = validate(contract, BIBLE.read_text(encoding="utf-8"),
                      INPUT.read_text(encoding="utf-8"))
    if errors:
        for error in errors:
            print("ERROR: " + error)
        raise SystemExit(1)
    print("1995 protagonist contract: static checks passed; no Unreal/animation verification claimed")


if __name__ == "__main__":
    main()
