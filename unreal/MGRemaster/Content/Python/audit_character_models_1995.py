"""Unreal Editor 5.7 character-audit and production block report.

Run in Editor (no network or automatic paid-asset import):
  Tools -> Execute Python Script -> Content/Python/audit_character_models_1995.py

Read-only to /Game. Writes JSON into Saved/Reports only. A found asset
is NOT a completed character: human visual review and gameplay capture remain
mandatory. This audit deliberately reports missing characters as FAIL, rather
than pretending imported meshes were created by an art-bible declaration.
"""
from __future__ import annotations

import json
from datetime import datetime, timezone
from pathlib import Path
import unreal


def project_root() -> Path:
    return Path(unreal.Paths.project_dir()).resolve().parents[1]


def main() -> None:
    manifest_path = project_root() / "data" / "CHARACTER_1995_REALISTIC_MODEL_PRODUCTION_GATE.json"
    if not manifest_path.is_file():
        raise FileNotFoundError(
            "Run the Unreal project inside this repository. The original "
            f"model manifest must exist at {manifest_path}."
        )
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    reports = []
    for entry in manifest["items"]:
        names = entry["assetPaths"]
        presence = {
            key: bool(unreal.EditorAssetLibrary.does_asset_exist(object_path))
            for key, object_path in names.items()
        }
        cls = {}
        for key in ("bodyMesh", "faceMesh"):
            if not presence[key]:
                cls[key] = "MISSING"
                continue
            asset = unreal.EditorAssetLibrary.load_asset(names[key])
            cls[key] = asset.get_class().get_name() if asset else "LOAD_FAILED"
        # An imported head and body may still have inappropriate anatomy,
        # LOD, UVs, skin, animation, hair, facial retarget or IP licensing.
        body_skeletal = cls.get("bodyMesh") == "SkeletalMesh"
        face_skeletal = cls.get("faceMesh") == "SkeletalMesh"
        blocking = [
            key for key, present in presence.items() if not present
        ]
        if presence["bodyMesh"] and not body_skeletal:
            blocking.append("bodyMesh_is_not_skeletal")
        if presence["faceMesh"] and not face_skeletal:
            blocking.append("faceMesh_is_not_skeletal")
        status = "ASSETS_PRESENT_EDITOR_REVIEW_REQUIRED" if not blocking else "BLOCKED_MISSING_OR_INVALID_ASSETS"
        reports.append({
            "id": entry["id"], "status": status,
            "sourcePaths": names, "objectPresence": presence,
            "assetClass": cls, "blockers": blocking,
            "facialCaptureVerified": False,
            "authoredCostumeApproved": False,
            "authoredActingReviewed": False,
            "1080pGameScreenshotRecorded": False,
            "productionReady": False,
        })

    blocked = sum(bool(r["blockers"]) for r in reports)
    result = {
        "createdAtUtc": datetime.now(timezone.utc).isoformat(),
        "engine": "Unreal Editor 5.7 or compatible; inspected live project asset registry",
        "manifestPath": str(manifest_path),
        "characterCount": len(reports),
        "modelAssetBlockers": blocked,
        "modelAssetsPresent": len(reports) - blocked,
        "productionReady": 0,
        "note": "Asset paths are not proof of likeness, legality, actual rig movement or visual polish.",
        "characters": reports,
    }
    out = Path(unreal.Paths.project_saved_dir()) / "Reports" / "CharacterCastAudit1995.json"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    unreal.log_warning(
        f"Metal Gear 1995 cast audit: {blocked}/{len(reports)} blocked by missing "
        f"mesh/face/AnimBP/Blueprint. Report: {out}"
    )


main()
