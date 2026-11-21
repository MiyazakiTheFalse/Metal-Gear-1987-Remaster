"""UE5.7 Editor-only, fail-closed audit of all imported character art sources.

Run with UnrealEditor-Cmd -run=pythonscript -script=<this-file>.
This verifies native assets, their skeletal compatibility, animation presence,
sample counts and imported centimetre scale. It does not prove good art, IK,
runtime gameplay, facial likeness or correct weapon contact. Never mutate art.
"""
from __future__ import annotations

import json
import math
import runpy
from pathlib import Path

import unreal

MANIFEST = runpy.run_path(
    str(Path(__file__).with_name("import_field_character_glbs.py")),
    run_name="mg_native_character_manifest",
)
ROLES = MANIFEST["ROLES"]
PREFIXES = MANIFEST["ROLE_PREFIXES"]
REQUIRED_CLIPS = MANIFEST["required_clips"]


def audit():
    records = []
    failures = []
    for role, (destination, name) in ROLES.items():
        target = f"{destination}/{name}"
        mesh = unreal.EditorAssetLibrary.load_asset(target)
        if not isinstance(mesh, unreal.SkeletalMesh):
            failures.append(f"{role}: native SkeletalMesh missing: {target}")
            continue
        skeleton = mesh.get_editor_property("skeleton")
        if skeleton is None:
            failures.append(f"{role}: native mesh has no Skeleton: {target}")
            continue

        # glTF stores metres; Unreal character actors use centimetres. A
        # 1.8cm imported body or a 180m giant may look like an invisible
        # player despite compiling and packaging successfully.
        bounds = mesh.get_bounds()
        extent = bounds.box_extent
        height_cm = float(extent.z) * 2.0
        if not math.isfinite(height_cm) or not 125.0 <= height_cm <= 240.0:
            failures.append(
                f"{role}: imported character height {height_cm:.3f}cm "
                "outside 125-240cm prototype range; inspect interchange "
                "unit conversion and origin, do not auto-scale in gameplay"
            )
        if not math.isfinite(float(extent.x)) or float(extent.x) < 5.0:
            failures.append(f"{role}: zero or corrupt render bounds: {target}")

        native_count = 0
        mesh_skeleton_path = skeleton.get_path_name()
        for clip in sorted(REQUIRED_CLIPS(role)):
            path = f"{destination}/Animations/AS_{PREFIXES[role]}_{clip}"
            sequence = unreal.EditorAssetLibrary.load_asset(path)
            if not isinstance(sequence, unreal.AnimSequence):
                failures.append(f"{role}: native animation missing: {path}")
                continue
            native_count += 1
            clip_skeleton = sequence.get_skeleton()
            if clip_skeleton is None or (
                clip_skeleton.get_path_name() != mesh_skeleton_path
            ):
                failures.append(
                    f"{role}/{clip}: animation is bound to the WRONG skeleton "
                    f"(expected {mesh_skeleton_path}, got "
                    f"{clip_skeleton.get_path_name() if clip_skeleton else None})"
                )
            try:
                frames = int(sequence.get_editor_property("number_of_sampled_frames"))
                seconds = float(sequence.get_editor_property("sequence_length"))
            except (ValueError, TypeError, AttributeError) as exc:
                failures.append(f"{role}/{clip}: cannot read animation frames: {exc}")
                continue
            if frames < 2 or not math.isfinite(seconds) or seconds < .10:
                failures.append(
                    f"{role}/{clip}: empty or malformed animation "
                    f"({frames} frames, {seconds:.3f} seconds)"
                )
        records.append({
            "role": role, "asset": target, "height_cm": round(height_cm, 2),
            "animation_clips": native_count,
            "expected_clips": len(REQUIRED_CLIPS(role)),
            "skeleton": mesh_skeleton_path,
        })
    summary = {
        "roles_expected": len(ROLES),
        "roles_validated": len(records),
        "native_clips_found": sum(x["animation_clips"] for x in records),
        "errors": failures,
        "note": "Native import audit only; not compiled/played/captured in PIE",
    }
    unreal.log("MG_NATIVE_CHARACTER_AUDIT_REPORT " + json.dumps(summary))
    for failure in failures:
        unreal.log_error("MG_NATIVE_CHARACTER_AUDIT_ERROR " + failure)
    if failures:
        raise RuntimeError(
            f"MG_NATIVE_CHARACTER_AUDIT_FAIL: {len(failures)} errors. "
            "Fix import/scale/animation compatibility before packaging."
        )
    # This is the only positive completion marker that the local PowerShell
    # packager accepts. A Python commandlet may log an exception yet return
    # process exit code zero, so checking only LASTEXITCODE is insufficient.
    unreal.log(
        f"MG_NATIVE_CHARACTER_AUDIT_PASS roles={len(records)} "
        f"clips={summary['native_clips_found']}"
    )
    return summary


if __name__ == "__main__":
    audit()
