#!/usr/bin/env python3
"""Source-aware event ledger checker; rejects fabricated J2ME verification.

This verifies metadata, not the truth of the underlying game sources.
"""
import argparse
import json
from pathlib import Path
import sys

VALID = {"comparative_pending_j2me", "j2me_verified", "remake_invention"}
MODES = {"interactive_dialogue", "radio_in_motion", "environmental_feedback",
         "short_cinematic_plus_playable_action", "cinematic", "ambient"}
CONTROL = {"retain", "retain_where_safe", "brief_lock_with_safe_restore"}
PERSIST = {"story", "communication", "hazard", "security", "interaction", "mission"}


def validate(data):
    errors = []
    if not isinstance(data, dict) or data.get("version") != 1:
        return ["version must be 1"]
    if not isinstance(data.get("target_edition"), str) or not data["target_edition"]:
        errors.append("target_edition is required")
    entries = data.get("entries")
    if not isinstance(entries, list) or not entries:
        return errors + ["entries must be nonempty"]
    seen = set()
    for index, e in enumerate(entries):
        if not isinstance(e, dict):
            errors.append(f"entries[{index}] must be an object")
            continue
        id = e.get("id")
        if not isinstance(id, str) or not id or id in seen:
            errors.append(f"entries[{index}] id missing or repeated")
        seen.add(id)
        status = e.get("evidence_status")
        if status not in VALID:
            errors.append(f"{id}: unknown evidence status")
        if e.get("type") not in PERSIST:
            errors.append(f"{id}: unknown event type")
        if not e.get("story_purpose") or not e.get("proposed_trigger"):
            errors.append(f"{id}: story purpose and proposed trigger required")
        if e.get("presentation") not in MODES or e.get("control_policy") not in CONTROL:
            errors.append(f"{id}: invalid presentation/control specification")
        if not e.get("interruption_policy") or not e.get("spoiler_gate"):
            errors.append(f"{id}: interruption and spoiler handling required")
        if not isinstance(e.get("requirements"), list):
            errors.append(f"{id}: requirements must be list")
        effects = e.get("persistent_effects")
        if not isinstance(effects, list) or any(not isinstance(x, str) or not x for x in effects):
            errors.append(f"{id}: malformed persistent effects")
        if status == "j2me_verified":
            if (not e.get("original_room") or not e.get("source_bytecode") or
                    not e.get("playback_comparison") or
                    e.get("requirements_status") != "verified_from_j2me"):
                errors.append(f"{id}: J2ME verification requires room, bytecode, playback and decoded gates")
        if status == "comparative_pending_j2me":
            if not e.get("source_url") or not e.get("source_game"):
                errors.append(f"{id}: comparative event must cite edition and source")
            if e.get("requirements_status") == "verified_from_j2me":
                errors.append(f"{id}: comparative event must not claim verified gates")
        if status == "remake_invention" and e.get("source_game"):
            errors.append(f"{id}: new events must not claim an original source-game event")
        if e.get("control_policy") == "brief_lock_with_safe_restore":
            if e.get("presentation") not in {"short_cinematic_plus_playable_action", "cinematic"}:
                errors.append(f"{id}: control lock only allowed for directed sequences")
    return errors


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("file", nargs="?", default="data/event_ledger.json")
    args = parser.parse_args()
    path = Path(args.file)
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as err:
        print("FAIL:", err)
        return 2
    errors = validate(data)
    for e in errors:
        print("FAIL:", e)
    if errors:
        return 1
    print(f"PASS: {len(data['entries'])} candidate event records have valid provenance controls; gameplay NOT certified")
    return 0


if __name__ == "__main__":
    sys.exit(main())
