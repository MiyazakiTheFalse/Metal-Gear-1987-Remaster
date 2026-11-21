"""Validate the source-first, full-campaign character narrative registry.

Checks documentation integrity and event ordering only, not gameplay completeness.
Usage: python tools/validate_character_arcs.py
"""
import json
from pathlib import Path

REGISTRY = Path(__file__).resolve().parents[1] / "data" / "character_arc_registry.json"

def validate(data):
    errors = []
    stages = {s["id"]: i for i, s in enumerate(data["stages"])}
    characters = {c["id"]: c for c in data["characters"]}
    beats = {b["id"]: b for b in data["beats"]}
    for group, raw, keyed in (
        ("stage", data["stages"], stages),
        ("character", data["characters"], characters),
        ("beat", data["beats"], beats),
    ):
        if len(raw) != len(keyed):
            errors.append(f"Duplicate {group} IDs")
    if data.get("dependency_semantics", "").startswith("Narrative ordering/context") is False:
        errors.append("Narrative and hard progression gates must be explicitly separated")
    if "j2me" not in data["authority"]["target"].lower():
        errors.append("Original J2ME edition lost target-authority precedence")
    if not data.get("gate_policy", {}).get("optional_radio", "").startswith("Optional exposition/advice radio never"):
        errors.append("Optional dialogue cannot gate progress; source-required Jennifer material support is an explicit exception")
    for c in data["characters"]:
        for field in ("outer_goal", "inner_conflict", "beginning", "turn", "end",
                      "story_boundary", "status"):
            if not c.get(field):
                errors.append(f"{c['id']} missing personal-story field {field}")
        for bid in c.get("beats", []):
            if bid not in beats:
                errors.append(f"{c['id']} refers to unknown beat {bid}")
            elif beats[bid]["character"] != c["id"]:
                errors.append(f"Character mismatch for {bid}")
    for b in data["beats"]:
        if b["character"] not in characters:
            errors.append(f"Beat {b['id']} missing character")
        if b["stage"] not in stages:
            errors.append(f"Beat {b['id']} missing stage")
        if not b.get("anchor") or not b.get("trigger"):
            errors.append(f"Beat {b['id']} needs a physical/radio anchor and a trigger")
        if b.get("provenance") == "j2me_verified":
            errors.append(f"Beat {b['id']} claims unverified J2ME provenance")
        for dep in b.get("depends_on", []):
            if dep not in beats:
                errors.append(f"{b['id']} unknown narrative prerequisite {dep}")
            elif b["stage"] in stages and beats[dep]["stage"] in stages:
                if stages[beats[dep]["stage"]] > stages[b["stage"]]:
                    errors.append(f"Backwards narrative dependency {dep} -> {b['id']}")
    for b in data["beats"]:
        if b["character"] in characters and b["id"] not in characters[b["character"]]["beats"]:
            errors.append(f"Beat {b['id']} absent from its character's arc")
    visited, active = set(), set()
    def visit(node):
        if node in active:
            errors.append(f"Narrative dependency cycle at {node}")
            return
        if node in visited:
            return
        active.add(node)
        for dep in beats[node].get("depends_on", []):
            if dep in beats:
                visit(dep)
        active.remove(node)
        visited.add(node)
    for node in beats:
        visit(node)
    required = ("solid_snake", "gray_fox", "big_boss", "schneider", "diane",
                "jennifer", "madnar", "ellen", "jennifer_brother",
                "shoot_gunner", "machinegun_kid", "fire_trooper", "coward_duck",
                "venom_snake", "steve", "tx11_arnolds", "fake_madnar",
                "pow_ensemble", "outer_heaven_security", "hind_d_crew",
                "armored_vehicles", "tx55")
    for cid in required:
        if cid not in characters:
            errors.append(f"Missing core character arc {cid}")
    required_beats = ("fox.missing", "fox.revelation", "madnar.condition",
                      "ellen.rescue", "madnar.weakness", "jennifer.brother_reveal",
                      "schneider.final_signal", "boss.abort_order",
                      "boss.reveal", "snake.escape")
    for bid in required_beats:
        if bid not in beats:
            errors.append(f"Missing core source-comparative narrative beat {bid}")
    ellen = beats.get("ellen.rescue", {})
    if ellen.get("anchor") != "building_1.basement.hidden_cell":
        errors.append("Lore regression: Ellen rescue must be Building One basement hidden cell")
    if "venus" in str(data).lower():
        errors.append("Unrelated identity typo")
    return errors

def main():
    data = json.loads(REGISTRY.read_text(encoding="utf-8"))
    errors = validate(data)
    if errors:
        for message in errors:
            print("FAIL:", message)
        raise SystemExit(1)
    print(f"PASS: {len(data['characters'])} arcs, {len(data['beats'])} beats, "
          f"{len(data['stages'])} phases; source and narrative links structurally sound")

if __name__ == "__main__":
    main()
