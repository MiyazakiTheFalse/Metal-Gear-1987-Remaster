"""Validate Doctrine 005 area contracts with no paid CI dependencies."""
from __future__ import annotations
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REGISTRY = ROOT / "data/area_contracts.json"
BEAT_REGISTRY = ROOT / "data/beat_contracts.json"
REQUIRED_AREAS = ("id", "stage", "beat_ids", "provenance", "verification_status",
                  "implementation_status", "constraints", "arrival", "topology",
                  "visual", "audio", "risk", "timing", "accessibility", "failure", "tests")
REQUIRED_NESTED = {
    "arrival": ("entry", "safe_observation", "first_goal", "landmark"),
    "topology": ("adjacency", "gates", "choices", "camera_clearance"),
    "visual": ("forewarning", "reveal", "spatial_readability"),
    "audio": ("sources", "occlusion", "non_audio_equivalent"),
    "risk": ("safe_points", "exposure_windows", "pressure"),
    "timing": ("trigger", "hint_policy", "lock_seconds"),
    "accessibility": ("color_independent", "caption_plan", "control_plan"),
    "failure": ("interrupts", "recovery"),
}
def validate(data: Any, beat_ids: set[str]) -> list[str]:
    errors: list[str] = []
    if not isinstance(data, dict):
        return ["Invalid registry object"]
    if data.get("schema_version") != 1 or data.get("doctrine") != "DOCTRINE_005":
        errors.append("Invalid area registry schema or doctrine")
    if data.get("supreme_doctrine") != "DOCTRINE_000" or data.get("scope") != "game_wide":
        errors.append("Area registry must be governed by global Doctrine 000")
    areas = data.get("areas")
    if not isinstance(areas, list) or not areas:
        return errors + ["areas must be a nonempty list"]
    seen: set[str] = set()
    for i, area in enumerate(areas):
        prefix = f"areas[{i}]"
        if not isinstance(area, dict):
            errors.append(prefix + " must be an object")
            continue
        for key in REQUIRED_AREAS:
            if key not in area or area[key] is None or area[key] == "":
                errors.append(prefix + " missing " + key)
        identifier = area.get("id")
        if not isinstance(identifier, str) or not identifier or " " in identifier:
            errors.append(prefix + " has invalid ID")
        elif identifier in seen:
            errors.append(prefix + " duplicates " + identifier)
        else:
            seen.add(identifier)
        for name, keys in REQUIRED_NESTED.items():
            parent = area.get(name)
            if not isinstance(parent, dict):
                errors.append(prefix + " missing nested " + name)
                continue
            for key in keys:
                if key not in parent or parent[key] is None or parent[key] == "":
                    errors.append(prefix + " missing " + name + "." + key)
        linked = area.get("beat_ids")
        if not isinstance(linked, list) or not linked:
            errors.append(prefix + " requires beat_ids")
        else:
            for beat_id in linked:
                if beat_id not in beat_ids:
                    errors.append(prefix + " references missing beat " + str(beat_id))
        if not isinstance(area.get("tests"), list) or not area.get("tests"):
            errors.append(prefix + " has no test routes")
        timing = area.get("timing")
        if isinstance(timing, dict):
            lock = timing.get("lock_seconds")
            if not isinstance(lock, (int, float)) or isinstance(lock, bool) or lock < 0:
                errors.append(prefix + " has invalid lock_seconds")
            elif lock > 0 and not area.get("forced_lock_justification"):
                errors.append(prefix + " forces undocumented loss of agency")
        access = area.get("accessibility")
        if isinstance(access, dict) and access.get("color_independent") is not True:
            errors.append(prefix + " requires non-colour-only cues")
        if area.get("implementation_status") == "shipped" and area.get("verification_status") != "packaged_playtested":
            errors.append(prefix + " falsely claims shippable implementation")
    return errors

def main() -> int:
    data = json.loads(REGISTRY.read_text(encoding="utf-8"))
    beats = json.loads(BEAT_REGISTRY.read_text(encoding="utf-8"))
    problems = validate(data, {b["id"] for b in beats["beats"]})
    for problem in problems:
        print("ERROR:", problem)
    if problems:
        return 1
    print(f"Area contracts valid: {len(data['areas'])} seed areas, NOT engine-tested.")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
