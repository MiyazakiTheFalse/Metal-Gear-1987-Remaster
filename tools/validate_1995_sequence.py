"""Structural cross-check of the 1995, 40-scene narrative sequence.

This is documentation QA, not Unreal playtesting or proof of source fidelity.
"""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SEQUENCE = ROOT / "data" / "1995_campaign_sequence.json"
REGISTRY = ROOT / "data" / "character_arc_registry.json"
INDIVIDUAL = ROOT / "data" / "1995_individual_character_timelines.json"


def validate(sequence, registry):
    errors = []
    steps = sequence["sequence"]
    stages = {stage["id"]: i for i, stage in enumerate(registry["stages"])}
    characters = {char["id"] for char in registry["characters"]}
    seen = set()
    preceding_stage = -1
    for expected_order, step in enumerate(steps, 1):
        step_id = step["id"]
        if step_id in seen:
            errors.append(f"Repeated sequence step {step_id}")
        seen.add(step_id)
        if step["order"] != expected_order:
            errors.append(f"Nonconsecutive order at {step_id}")
        if step["phase"] not in stages:
            errors.append(f"Unknown stage at {step_id}")
        else:
            if stages[step["phase"]] < preceding_stage:
                errors.append(f"Regressing phase at {step_id}")
            preceding_stage = stages[step["phase"]]
        for ch in step["cast"]:
            if ch not in characters:
                errors.append(f"Unknown cast member {ch} in {step_id}")
        if step.get("exact_1995_time") != "not_established_by_source":
            errors.append(f"Invented timestamp at {step_id}")
        if step.get("grade") == "j2me_verified":
            errors.append(f"Unverified source claim at {step_id}")
        if not step.get("player_beat") or not step.get("dramatic_role"):
            errors.append(f"No authored player and dramatic function at {step_id}")
        for dependent in step.get("dependency", []):
            if dependent not in seen:
                errors.append(f"Invalid earlier dependency {dependent} in {step_id}")
    if sequence["year"] != 1995:
        errors.append("Wrong in-universe year")
    if "j2me" not in sequence["authoritative_edition"].lower():
        errors.append("J2ME target edition lost precedence")
    if len(steps) < 35:
        errors.append("The sequence is no longer campaign-wide")
    by_id = {s["id"]: s for s in steps}
    must = ["N00_PRELUDE", "N08_CELL_BREACH", "N09_FOX_TRUTH",
            "N20_ACTUAL_MADNAR", "N22_HIDDEN_ELLEN", "N24_RETURN_TO_SCIENTIST",
            "N29_SCHNEIDER_LAST_CALL", "N30_COWARD_DUCK_ROOM",
            "N31_BROTHER_FREED", "N32_OWNERSHIP_REVEALED",
            "N35_TX55_SABOTAGE", "N36_SELF_DESTRUCT",
            "N37_FINAL_BIG_BOSS", "N38_ESCAPE_LEFT_LADDER", "N39_CLOSING_BROADCAST"]
    for name in must:
        if name not in by_id:
            errors.append(f"Missing story anchor: {name}")
    if all(name in by_id for name in must):
        at = {name: by_id[name]["order"] for name in must}
        for before, after in [
            ("N09_FOX_TRUTH", "N20_ACTUAL_MADNAR"),
            ("N20_ACTUAL_MADNAR", "N22_HIDDEN_ELLEN"),
            ("N22_HIDDEN_ELLEN", "N24_RETURN_TO_SCIENTIST"),
            ("N30_COWARD_DUCK_ROOM", "N31_BROTHER_FREED"),
            ("N32_OWNERSHIP_REVEALED", "N35_TX55_SABOTAGE"),
            ("N35_TX55_SABOTAGE", "N36_SELF_DESTRUCT"),
            ("N36_SELF_DESTRUCT", "N37_FINAL_BIG_BOSS"),
            ("N37_FINAL_BIG_BOSS", "N38_ESCAPE_LEFT_LADDER"),
        ]:
            if at[before] >= at[after]:
                errors.append(f"Broken 1995 story order: {before} before {after}")
        if "building_1" not in by_id["N22_HIDDEN_ELLEN"]["location"]:
            errors.append("Ellen rescue moved out of Building One basement")
        if "building_2" not in by_id["N24_RETURN_TO_SCIENTIST"]["location"]:
            errors.append("Madnar return no longer in Building Two")
    if "venom_snake" not in by_id.get("N37_FINAL_BIG_BOSS", {}).get("cast", []):
        errors.append("Missing later-canon phantom identity distinction")
    if "venom_snake" in by_id.get("N01_INSERTION", {}).get("cast", []):
        errors.append("Phantom incorrectly exposed at game start")
    for char in characters:
        if not any(char in step["cast"] for step in steps):
            errors.append(f"Character/group not linked to any campaign scene: {char}")
    return errors


def validate_individual(individual, sequence, registry):
    errors = []
    by_id = {step["id"]: step for step in sequence["sequence"]}
    by_char = {c["id"]: c for c in registry["characters"]}
    timelines = individual["character_timelines"]
    if len(timelines) != len(by_char):
        errors.append("Individual timeline coverage differs from cast registry")
    for person in timelines:
        cid = person["id"]
        if cid not in by_char:
            errors.append(f"Unknown individual timeline: {cid}")
            continue
        actual = [step for step in sequence["sequence"] if cid in step["cast"]]
        declared = person["sequence"]
        if [s["scene_id"] for s in declared] != [s["id"] for s in actual]:
            errors.append(f"Lost or reordered story reference for {cid}")
        if [s["order"] for s in declared] != sorted(s["order"] for s in declared):
            errors.append(f"Nonchronological individual story for {cid}")
        if person["final_1995_status"] != by_char[cid]["end"]:
            errors.append(f"Final 1995 state diverged for {cid}")
        if person["beat_ids"] != by_char[cid]["beats"]:
            errors.append(f"Individual beat IDs diverged for {cid}")
    return errors


def main():
    sequence = json.loads(SEQUENCE.read_text(encoding="utf-8"))
    registry = json.loads(REGISTRY.read_text(encoding="utf-8"))
    individual = json.loads(INDIVIDUAL.read_text(encoding="utf-8"))
    errors = validate(sequence, registry) + validate_individual(individual, sequence, registry)
    if errors:
        for issue in errors:
            print("FAIL:", issue)
        raise SystemExit(1)
    print(f"PASS: {len(sequence['sequence'])} scenes span "
          f"{len(registry['characters'])} character/group arcs in 1995")


if __name__ == "__main__":
    main()
