#!/usr/bin/env python3
"""Audit the complete campaign ledger without treating design/source as playtested."""
from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path

DEFAULT_LEDGER = Path(__file__).resolve().parents[1] / "data" / "FULL_CAMPAIGN_BEAT_BY_BEAT_IMPLEMENTATION_LEDGER.json"
RANK = {"not_started": 0, "design_only": 1, "source_prototype": 2,
        "running_in_engine": 3, "playtested": 4, "production_ready": 5}
RUNTIME_DIMENSIONS = ("browser_runtime", "unreal_runtime")


def audit(data: dict) -> tuple[list[str], dict]:
    errors: list[str] = []
    missions = data.get("missions", [])
    dimensions = data.get("dimensions", [])
    if not isinstance(missions, list) or not missions:
        return ["missions must be a nonempty list"], {}
    if not isinstance(dimensions, list) or len(dimensions) != len(set(dimensions)):
        return ["dimensions must be a unique list"], {}
    for dimension in RUNTIME_DIMENSIONS:
        if dimension not in dimensions:
            errors.append("missing runtime dimension: " + dimension)
    seen_missions, seen_beats = set(), set()
    totals = Counter()
    per_mission = []
    mismatches = []
    for mission in missions:
        mid = mission.get("id")
        if not isinstance(mid, str) or not mid:
            errors.append("mission missing id")
            continue
        if mid in seen_missions:
            errors.append(f"duplicate mission id: {mid}")
        seen_missions.add(mid)
        beats = mission.get("beats", [])
        if not isinstance(beats, list) or not beats:
            errors.append(f"{mid}: missing beats")
            continue
        local_ordinals = set()
        for beat in beats:
            bid = beat.get("id")
            if not isinstance(bid, str) or not bid:
                errors.append(f"{mid}: beat missing id")
                continue
            if bid in seen_beats:
                errors.append(f"duplicate beat id: {bid}")
            seen_beats.add(bid)
            ordinal = beat.get("ordinal")
            if not isinstance(ordinal, int) or isinstance(ordinal, bool) or ordinal < 1:
                errors.append(f"{bid}: ordinal must be positive integer")
            elif ordinal in local_ordinals:
                errors.append(f"{bid}: duplicate ordinal {ordinal}")
            local_ordinals.add(ordinal)
            overall = beat.get("overall")
            if overall not in RANK:
                errors.append(f"{bid}: invalid overall status {overall!r}")
                continue
            totals[overall] += 1
            completeness = beat.get("completeness", {})
            if not isinstance(completeness, dict):
                errors.append(f"{bid}: completeness must be an object")
                continue
            for dim in dimensions:
                if dim not in completeness:
                    errors.append(f"{bid}: missing dimension {dim}")
            browser = completeness.get("browser_runtime")
            unreal = completeness.get("unreal_runtime")
            for name, value in (("browser_runtime", browser), ("unreal_runtime", unreal)):
                if value not in RANK:
                    errors.append(f"{bid}: invalid {name} status {value!r}")
            if browser in RANK and unreal in RANK and browser != unreal:
                mismatches.append({"beat": bid, "browser": browser, "unreal": unreal})
            # No completed/playtested claim can precede runtime and hands-on evidence.
            if overall in ("playtested", "production_ready"):
                if not beat.get("test_evidence") and not completeness.get("player_playtest") in ("playtested", "production_ready"):
                    errors.append(f"{bid}: claimed {overall} without player test evidence")
                if any(completeness.get(d) not in ("running_in_engine", "playtested", "production_ready") for d in RUNTIME_DIMENSIONS):
                    errors.append(f"{bid}: claimed {overall} without both runtime integrations")
            if overall == "production_ready" and any(completeness.get(d) != "production_ready" for d in RUNTIME_DIMENSIONS):
                errors.append(f"{bid}: production ready requires both runtime dimensions production ready")
        per_mission.append({"id": mid, "title": mission.get("title", ""), "beats": len(beats),
                            "statuses": dict(Counter(b.get("overall") for b in beats))})
    result = {"mission_count": len(missions), "beat_count": sum(totals.values()),
              "overall_statuses": dict(totals), "missions": per_mission,
              "runtime_status_mismatches": mismatches,
              "runtime_mismatch_count": len(mismatches),
              "unverified_run_notice": "Static ledger audit only: no browser, Unreal or player tests executed."}
    return errors, result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("ledger", nargs="?", type=Path, default=DEFAULT_LEDGER)
    parser.add_argument("--json", action="store_true", help="emit machine-readable report")
    args = parser.parse_args()
    data = json.loads(args.ledger.read_text(encoding="utf-8"))
    errors, report = audit(data)
    if args.json:
        print(json.dumps({"ok": not errors, "errors": errors, **report}, indent=2))
    else:
        print(f"{report.get('mission_count', 0)} missions / {report.get('beat_count', 0)} beats")
        print("Recorded beat stages:", report.get("overall_statuses", {}))
        print("Browser / Unreal source-stage mismatches:", report.get("runtime_mismatch_count", 0))
        for error in errors:
            print("ERROR:", error)
        print("Static audit only; never substitute for a build or playtest.")
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
