"""Validate Doctrine 004 contracts (stdlib; no paid CI or engine required)."""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REGISTRY = ROOT / "data/beat_contracts.json"
REQUIRED_BEAT = ("id", "kind", "provenance", "intent", "trigger", "action",
                 "visual", "audio", "timing", "state", "interruptions",
                 "verification", "implementation_status", "verification_status")
REQUIRED_NESTED = {
    "visual": ("foreshadow", "impact", "aftermath"),
    "audio": ("origin", "response", "non_audio_equivalent"),
    "timing": ("start_event", "feedback_target_ms", "max_forced_lock_seconds",
               "optional_hint_seconds", "timing_status"),
    "state": ("transaction", "persistence", "ai_knowledge"),
}
VALID_IMPLEMENTATION = {"design_only", "source_prototype", "playtest_candidate", "shipped"}
VALID_VERIFICATION = {"unverified", "source_checked", "engine_tested", "packaged_playtested"}

def validate_registry(data: Any) -> list[str]:
    errors: list[str] = []
    if not isinstance(data, dict):
        return ["Root must be a JSON object"]
    if data.get("schema_version") != 1:
        errors.append("schema_version must be 1")
    if data.get("doctrine") != "DOCTRINE_004" or data.get("scope") != "game_wide":
        errors.append("Binding game-wide doctrine identifier missing")
    beats = data.get("beats")
    if not isinstance(beats, list) or not beats:
        return errors + ["beats must be a nonempty list"]
    seen: set[str] = set()
    for index, beat in enumerate(beats):
        prefix = "beats[{}]".format(index)
        if not isinstance(beat, dict):
            errors.append(prefix + ": expected object")
            continue
        for key in REQUIRED_BEAT:
            value = beat.get(key)
            if value is None or value == "" or value == []:
                if key != "interruptions":
                    errors.append(prefix + ": missing " + key)
        identifier = beat.get("id")
        if not isinstance(identifier, str) or not identifier or " " in identifier:
            errors.append(prefix + ": invalid stable id")
        elif identifier in seen:
            errors.append(prefix + ": duplicate id " + identifier)
        else:
            seen.add(identifier)
        for parent, keys in REQUIRED_NESTED.items():
            value = beat.get(parent)
            if not isinstance(value, dict):
                errors.append(prefix + ": " + parent + " must be object")
                continue
            for child in keys:
                member = value.get(child)
                if member is None or member == "":
                    errors.append(prefix + ": missing " + parent + "." + child)
        timing = beat.get("timing")
        if isinstance(timing, dict):
            latency = timing.get("feedback_target_ms")
            lock = timing.get("max_forced_lock_seconds")
            hints = timing.get("optional_hint_seconds")
            if not isinstance(latency, (int, float)) or isinstance(latency, bool) or latency <= 0:
                errors.append(prefix + ": feedback_target_ms must be positive")
            if not isinstance(lock, (int, float)) or isinstance(lock, bool) or lock < 0:
                errors.append(prefix + ": max_forced_lock_seconds must be nonnegative")
            elif lock > 0 and not beat.get("forced_lock_justification"):
                errors.append(prefix + ": forced control loss requires justification")
            if not isinstance(hints, list) or any(
                not isinstance(v, (int, float)) or isinstance(v, bool) or v <= 0
                for v in hints
            ) or hints != sorted(set(hints)):
                errors.append(prefix + ": optional hints must be unique ascending positive seconds")
        if beat.get("implementation_status") not in VALID_IMPLEMENTATION:
            errors.append(prefix + ": invalid implementation_status")
        verification = beat.get("verification_status")
        if verification not in VALID_VERIFICATION:
            errors.append(prefix + ": invalid verification_status")
        if verification in ("engine_tested", "packaged_playtested") and not beat.get("test_evidence"):
            errors.append(prefix + ": verified claims require concrete test_evidence")
        if beat.get("implementation_status") == "shipped" and verification != "packaged_playtested":
            errors.append(prefix + ": shipped requires packaged_playtested")
        for key in ("interruptions", "verification"):
            if not isinstance(beat.get(key), list) or not beat.get(key):
                errors.append(prefix + ": " + key + " needs explicit scenarios")
    return errors

def main() -> int:
    data = json.loads(REGISTRY.read_text(encoding="utf-8"))
    errors = validate_registry(data)
    for problem in errors:
        print("ERROR:", problem)
    if errors:
        print("Beat contract validation failed: {} issue(s)".format(len(errors)))
        return 1
    print("Beat contract validation passed: {} registered beat(s); not UE playtested.".format(len(data["beats"])))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
